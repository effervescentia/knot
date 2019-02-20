open Core;

type t = {
  is_complete: unit => bool,
  resolve: resolve_target => unit,
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

  let rec attempt_resolve = x =>
    try (
      {
        ignore(x);

        if (List.length(resolve_queue^) == 0) {
          is_resolving := false;
        } else {
          let fst = List.hd(resolve_queue^);
          resolve_queue := List.tl(resolve_queue^);
          attempt_resolve(fst);
        };
      }
    ) {
    | _ => attempted_queue := [x, ...attempted_queue^]
    };

  {
    is_complete: () =>
      List.length(resolve_queue^) == 0 && List.length(attempted_queue^) == 0,
    resolve: x => {
      if (Resolver_Static.resolve(x)) {
        ();
      } else if (is_resolving^) {
        resolve_queue := [x, ...resolve_queue^];
      } else {
        is_resolving := true;
        attempt_resolve(x);
      };

      /* Debug.print_resolve_target(x)
         |> Printf.sprintf("resolving %s")
         |> print_endline; */

      ();
    },
  };
};
