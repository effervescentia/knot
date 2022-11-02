let is_main: Result.export_t => bool =
  fun
  | MainExport(_) => true
  | NamedExport(_) => false;
