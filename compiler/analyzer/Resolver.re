open Core;

type t = {
  is_complete: unit => bool,
  resolve: (NestedHashtbl.t(string, member_type), resolve_target) => unit,
  pending: unit => list(resolve_target),
};

let of_module = m => ModuleScope(m);
let of_declaration = d => DeclarationScope(d);
let of_import = (module_, i) => ImportScope(module_, i);
let of_parameter = p => ParameterScope(p);
let of_property = p => PropertyScope(p);
let of_expression = e => ExpressionScope(e);
let of_reference = r => ReferenceScope(r);

let create = module_tbl => {
  let resolve_queue = ref([]);
  let attempted_queue = ref([]);
  let is_resolving = ref(false);

  {
    is_complete: () =>
      List.length(resolve_queue^) == 0 && List.length(attempted_queue^) == 0,
    resolve: (symbol_tbl, x) => {
      let implicit_resolve =
        Resolver_Implicit.create(
          resolve_queue,
          attempted_queue,
          is_resolving,
        );

      Debug.print_resolve_target(x)
      |> Printf.sprintf("resolving : %s")
      |> print_endline;

      if (Resolver_Explicit.resolve(module_tbl, symbol_tbl, x)) {
        ();
      } else if (is_resolving^) {
        resolve_queue := [x, ...resolve_queue^];
      } else {
        is_resolving := true;
        implicit_resolve(x);
      };
    },
    pending: () => attempted_queue^ @ resolve_queue^,
  };
};