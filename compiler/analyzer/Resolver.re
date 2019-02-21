open Core;

type t = {
  is_complete: unit => bool,
  resolve: resolve_target => unit,
  pending: unit => list(resolve_target),
};

let of_module = m => ModuleScope(m);
let of_declaration = d => DeclarationScope(d);
let of_import = i => ImportScope(i);
let of_expression = e => ExpressionScope(e);
let of_reference = r => ReferenceScope(r);
let of_jsx = j => JSXScope(j);

let create = () => {
  let resolve_queue = ref([]);
  let attempted_queue = ref([]);
  let is_resolving = ref(false);

  {
    is_complete: () =>
      List.length(resolve_queue^) == 0 && List.length(attempted_queue^) == 0,
    resolve: x => {
      let dynamic_resolve =
        Resolver_Dynamic.create(resolve_queue, attempted_queue, is_resolving);

      if (Resolver_Static.resolve(x)) {
        ();
      } else if (is_resolving^) {
        resolve_queue := [x, ...resolve_queue^];
      } else {
        is_resolving := true;
        dynamic_resolve(x);
      };
    },
    pending: () => attempted_queue^ @ resolve_queue^,
  };
};
