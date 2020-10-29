let default = default_value =>
  fun
  | Some(value) => value
  | None => default_value;
