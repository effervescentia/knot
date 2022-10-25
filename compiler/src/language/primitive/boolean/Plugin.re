open Knot.Kore;

let type_ = `Boolean;

let parse = Parser.boolean;

let analyze = x => Node.add_type(Type.Valid(type_), x);

let pp = Formatter.pp_boolean;

let dump = pp;
