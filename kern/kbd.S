.global kbd_wrapper

kbd_wrapper:
  pusha
  call kbd_handler
  popa
  iret
