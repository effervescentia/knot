open Knot.Kore;

exception UnknownTypeEncountered;

type module_entry_t =
  | Type
  | Value;

type valid_t =
  | Nil
  | Boolean
  | Integer
  | Float
  | String
  | Element
  | Style
  | List(t)
  | Object(list((string, (t, bool))))
  | Enumerated(list((string, list(t))))
  | Function(list(t), t)
  | View(list((string, (t, bool))), t)
  | Module(list((string, module_entry_t, t)))

and invalid_t =
  | NotInferrable

and t =
  | Valid(valid_t)
  | Invalid(invalid_t);
