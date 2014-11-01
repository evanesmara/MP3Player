
   .file        "a7hal_a.S"

  .text
  .section .text
  .code 32

@******************************************************************************
@                            CONSTANT DECLARATIONS
@******************************************************************************

#define NO_INT   0xC0
#define MODE_SYS 0x1F
#define MODE_IRQ 0x12


@******************************************************************************
@                            EXTERNAL DECLARATIONS
@******************************************************************************
  .extern  osISRExit_oshal
  .extern  handleIRQs_oshal

  .extern  pRunProc
addr_pRunProc:
  .long    pRunProc

  .extern  pNxtToRun
addr_pNxtToRun:
  .long    pNxtToRun

  .extern  isrNesting
addr_isrNesting:
  .long    isrNesting


@******************************************************************************
@ Description:
@   This routine is called by osStart to start the first process. It is
@   assumed that "pNxtToRun" is pointing at the process control block(PCB) 
@   of the first process to start. "pRunProc" is alterered to point at the 
@   newly started process' PCB.
@
@   The stack frame must have been initialized by stkFrameInit.
@
@ Note:
@   The stack pointer is located first in the PCB and therefore it can be
@   read and write using the PCB pointer directly.
@******************************************************************************
        .align 2
        .global startHighProc_oshal
        .type startHighProc_oshal,function
startHighProc_oshal:
        @
        @ Swicth to SYS mode and disable IRQ and FIQ
        @
        MSR     CPSR_cxsf,#(NO_INT | MODE_SYS)
        
        @
        @ Switch in the highest priority process
        @
        B       ctxSwitchIsr_oshal


@******************************************************************************
@ Description:
@   This routine is called from schedule to make a context switch.
@   "pRunProc" is assumed to point at the currently running process and 
@   "pNxtToRun" is assumed to point at the PCB of the process to switch to.
@
@ Note:
@   isrCtxSwitch is used when the context switch is made after an 
@   interrupt service routine (isr)
@******************************************************************************
        .align 2
        .global ctxSwitch_oshal
        .type ctxSwitch_oshal,function

        .global ctxSwitchIsr_oshal
        .type ctxSwitchIsr_oshal,function

ctxSwitch_oshal:
        @
        @ Store current context
        @
        STMFD   SP!,{LR}                @ Push PC (LR should be pushed in place of PC)
        STMFD   SP!,{R0-R12,LR}         @ Push LR & register file
        MRS     R4,CPSR
        ORR     R4,R4,#0x20             @ Set THUMB-bit in order to return to THUMB code
        STMFD   SP!,{R4}                @ Push current psr (CPSR)

        @
        @ Store SP of the (to be) pre-empted process (*pRunProc = SP)
        @
        LDR     R1,addr_pRunProc        @ R1 = &pRunProc
        LDR     R2,[R1]                 @ R2 = pRunProc
        STR     SP,[R2]                 @ Store SP of the (to be) pre-empted process (SP -> [pRunProc]) 

ctxSwitchIsr_oshal:
        @
        @ pRunProc = pNxtToRun
        @
        LDR     R0,addr_pNxtToRun       @ R0 = &pNxtToRun         
        LDR     R1,addr_pRunProc        @ R1 = &pRunProc
        LDR     R3,[R0]                 @ R3 = pNxtToRun
        STR     R3,[R1]                 @ pRunProc = pNxtToRun (via R3) 

        @
        @ Retrieve SP of process to run (SP = *pRunProc)
        @
        LDR     SP,[R3]                 @ Get SP of the new process to run ([pNxtToRun] -> SP) 
        
        @
        @ Retrieve context of process to run
        @
        LDMFD   SP!,{R4}                @ Pop new CPSR
        MSR     CPSR_cxsf,R4            @
        LDMFD   SP!,{R0-R12,LR,PC}      @ Pop new task's R0-R12, LR & PC


@******************************************************************************
@ Description:
@   A general interrupt handler for OS-aware interrupts.
@   Save the current context on the stack in the way that the OS context switch
@   handler likes to have it. Allow interrupts to make use of the available
@   functions in the OS.
@******************************************************************************
        .align 2
        .global generalIRQ_oshal
        .type generalIRQ_oshal,function

generalIRQ_oshal:
        STMFD   SP!,{R1-R3}
        MOV     R1,SP
        ADD     SP,SP,#12
        SUB     R2,LR,#4
        MRS     R3,SPSR
        
        @
        @ Disable interrupts and change to SYSTEM mode (from IRQ mode)
        @
        MSR     CPSR_c,#(NO_INT | MODE_SYS)
        
        @
        @ Save interrupted process contex
        @
        STMFD   SP!,{R2}                @ Push return PC, which has been adjusted above
        STMFD   SP!,{R4-R12,LR}

        LDMFD   R1!,{R4-R6}             @ Move R1-R3 from IRQ stack to SYS stack
        STMFD   SP!,{R4-R6}
        STMFD   SP!,{R0}                @ Push R0
        STMFD   SP!,{R3}                @ Push CPSR (actually IRQ's SPSR)

        @
        @ isrNesting++  =>  block scheduling during interrupts
        @
        LDR     R0,addr_isrNesting      @ R0 = &isrNesting         
        LDRB    R1,[R0]                 @ R1 = isrNesting
        ADD     R1,R1,#1                @ R1 = R1 + 1
        STRB    R1,[R0]                 @ Store new value of 'isrNesting' 

        @
        @ Store SP if (isrNesting == 1)
        @
        CMP     R1,#1
        BNE     generalIRQ_cont_oshal
        
        @
        @ Store SP of the process that may be pre-empted after this ISR (SP -> [pRunProc]
        @
        LDR     R4,addr_pRunProc        @ R4 = &pRunProc
        LDR     R5,[R4]                 @ R5 = pRunProc
        STR     SP,[R5]                 @ Store SP of the to be (maybe) pre-empted process

generalIRQ_cont_oshal:
        @
        @ Change to IRQ mode (from SYSTEM mode)
        @
        MSR     CPSR_c,#(NO_INT | MODE_IRQ)

        @
        @ Handle the IRQ by calling the appropriate ISR
        @ (that will execute in IRQ mode)
        @
        BL      handleIRQs_oshal

        @
        @ Change back to SYSTEM mode (from IRQ mode)
        @
        MSR     CPSR_c,#(NO_INT | MODE_SYS)
        
        @
        @ Signal to the OS that ISR has reached its end
        @ Check if time to perform a context switch.
        @
        BL      osISRExit
        
        @
        @ Restore interrupted process context and return
        @
        LDMFD   SP!,{R4}
        MSR     CPSR_cxsf,R4
        LDMFD   SP!,{R0-R12,LR,PC}      @ Restore registers of interrupted context

