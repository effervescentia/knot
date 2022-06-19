open Infix;

type lookup_t = {
  types: Hashtbl.t(string, Type.t),
  values: Hashtbl.t(string, Type.t),
};

type t = {
  imported: lookup_t,
  declared: lookup_t,
};

/* static */

let create = (): t => {
  imported: {
    types: Hashtbl.create(0),
    values: Hashtbl.create(0),
  },
  declared: {
    types: Hashtbl.create(0),
    values: Hashtbl.create(0),
  },
};

/* methods */

let import = (id: string, type_: Type.t, table: t) =>
  Hashtbl.replace(table.imported.values, id, type_);

let define_type = (id: string, type_: Type.t, table: t) =>
  Hashtbl.replace(table.declared.types, id, type_);

let define_value = (id: string, type_: Type.t, table: t) =>
  Hashtbl.replace(table.declared.values, id, type_);

let resolve_type = (id: string, table: t) =>
  Hashtbl.find_opt(table.declared.types, id)
  |?| Hashtbl.find_opt(table.imported.types, id);

let resolve_value = (id: string, table: t) =>
  Hashtbl.find_opt(table.declared.types, id)
  |?| Hashtbl.find_opt(table.imported.types, id);
