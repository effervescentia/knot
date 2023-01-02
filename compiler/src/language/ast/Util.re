let is_main: Module.export_t => bool =
  fun
  | Main => true
  | Named => false;
