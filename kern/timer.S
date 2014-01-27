.global timer_wrapper

timer_wrapper:
  pusha
  call timer_handler
  popa
  iret
