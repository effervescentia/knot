open Kore;

let is_main: A.export_t => bool =
  fun
  | MainExport(_) => true
  | NamedExport(_) => false;
