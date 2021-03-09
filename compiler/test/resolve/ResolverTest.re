open Kore;

module Resolver = Resolve.Resolver;

let __cache = "foo";
let __root_dir = "bar";
let __path = "fizz.txt";

let suite =
  "Resolve.Resolver"
  >::: [
    "create()"
    >: (
      () => {
        Assert.resolver(
          {cache: __cache, root_dir: __root_dir},
          Resolver.create(__cache, __root_dir),
        );
      }
    ),
    "resolve_module() - resolve from cache"
    >: (
      () => {
        let resolver = Resolver.create(__cache, __root_dir);

        Assert.module_(
          Resolve.Module.File({
            relative: __path,
            full: Filename.concat(__cache, __path),
          }),
          Resolver.resolve_module(Internal(__path), resolver),
        );
      }
    ),
    "resolve_module() - resolve from source"
    >: (
      () => {
        let resolver = Resolver.create(__cache, __root_dir);

        Assert.module_(
          Resolve.Module.File({
            relative: __path,
            full: Filename.concat(__root_dir, __path),
          }),
          Resolver.resolve_module(
            ~skip_cache=true,
            Internal(__path),
            resolver,
          ),
        );
      }
    ),
    "resolve_module() - resolve external"
    >: (
      () => {
        let resolver = Resolver.create(__cache, __root_dir);

        Alcotest.check_raises(
          "should throw NotImplemented exception", NotImplemented, () =>
          Resolver.resolve_module(
            ~skip_cache=true,
            External(__path),
            resolver,
          )
          |> ignore
        );
      }
    ),
  ];
