open Core;

type t = {
  pop: unit => option(resolve_target),
  reject: resolve_target => unit,
  finish: unit => unit,
};

let rec create = (resolve_queue, attempted_queue, is_resolving) =>
  resolve({
    pop: () =>
      if (List.length(resolve_queue^) == 0) {
        None;
      } else {
        let next = List.hd(resolve_queue^);
        resolve_queue := List.tl(resolve_queue^);

        Some(next);
      },
    reject: target => attempted_queue := [target, ...attempted_queue^],
    finish: () => is_resolving := false,
  })
and resolve = ({reject, pop, finish} as api, x) =>
  try (
    {
      raise(NotImplemented);

      switch (pop()) {
      | Some(next) => resolve(api, next)
      | None => finish()
      };
    }
  ) {
  | _ => reject(x)
  };
