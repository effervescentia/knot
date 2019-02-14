open Core;

type t('a, 'b, 'c) = {
  is_complete: unit => bool,
  resolve: 'c => unit,
  add: ('a, 'b) => unit,
};

let create = symbol_tbl => {
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
    resolve: x =>
      if (is_resolving^) {
        resolve_queue := [x, ...resolve_queue^];
      } else {
        is_resolving := true;
        attempt_resolve(x);
      },
    add: (key, value) => Hashtbl.add(symbol_tbl, key, value),
  };
};