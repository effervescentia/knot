include Knot.Kore;

type js_module_t =
  | Common
  | ES6;

type target_t =
  | JavaScript(js_module_t)
  | Knot;
