type module_defn =
  | ModuleDefn(list(decl_defn))
and decl_defn =
  | MainDefn(type_defn)
and type_defn =
  | BooleanDefn
  | NumberDefn
  | StringDefn
  | JSXDefn
  | NilDefn
  | ObjectDefn(Hashtbl.t(string, type_defn))
  | FunctionDefn(list(type_defn), type_defn);
