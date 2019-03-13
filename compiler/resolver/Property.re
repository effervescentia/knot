open Core;
open NestedHashtbl;

let resolve = (((name, type_def, default_val), _) as promise) =>
  switch (type_def, default_val) {
  | (
      Some((_, {contents: Resolved(l_type)})),
      Some((_, {contents: Resolved(r_type)})),
    )
      when l_type !== r_type =>
    raise(DefaultValueTypeMismatch)
  | (None, None) =>
    snd(promise) := Synthetic(Any_t, []);
    true;
  | (Some((_, {contents: Resolved(typ)})), _)
  | (_, Some((_, {contents: Resolved(typ)}))) =>
    snd(promise) := Resolved(typ);
    true;
  | _ => false
  };

let resolve_param = (symbol_tbl, ((name, _, _), _) as promise) =>
  if (resolve(promise)) {
    symbol_tbl.add(name, Util.get_type(promise));
    true;
  } else {
    false;
  };
