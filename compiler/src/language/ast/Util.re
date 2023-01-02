let is_main: Module.export_kind_t => bool =
  fun
  | Main => true
  | Named => false;
