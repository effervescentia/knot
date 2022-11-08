let is_main: Module.export_t => bool =
  fun
  | MainExport(_) => true
  | NamedExport(_) => false;
