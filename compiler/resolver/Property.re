open Core;
open NestedHashtbl;

let resolve = (promise, (name, type_def, default_val)) =>
  switch (type_def, default_val) {
  | (
      Some((_, {contents: Resolved(l_type)})),
      Some((_, {contents: Resolved(r_type)})),
    )
      when l_type !== r_type =>
    raise(DefaultValueTypeMismatch)
  | (None, None) => Some(Util.generate_any_type())
  | (Some((_, {contents: Resolved(typ)})), _)
  | (_, Some((_, {contents: Resolved(typ)}))) => Some(typ)
  | _ => None
  };

let resolve_param = (symbol_tbl, promise, (name, _, _) as param) =>
  resolve(promise, param)
  |> (
    fun
    | Some(typ) as res => {
        symbol_tbl.add(name, typ);

        res;
      }
    | None => None
  );
