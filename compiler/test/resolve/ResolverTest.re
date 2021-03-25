open Kore;

module Resolver = Resolve.Resolver;

let __cache = "foo";
let __root_dir = "bar";
let __source_dir = "fizz";
let __id = Reference.Namespace.Internal("buzz");
let __path = "buzz.kn";

let suite =
  "Resolve.Resolver"
  >::: [
    "create()"
    >: (
      () => {
        Assert.resolver(
          {cache: __cache, root_dir: __root_dir, source_dir: __source_dir},
          Resolver.create(__cache, __root_dir, __source_dir),
        );
      }
    ),
    "resolve_module() - resolve from cache"
    >: (
      () => {
        let resolver = Resolver.create(__cache, __root_dir, __source_dir);
        let relative = Filename.concat(__source_dir, __path);

        Assert.module_(
          Resolve.Module.File({
            relative,
            full: Filename.concat(__cache, relative),
          }),
          Resolver.resolve_module(__id, resolver),
        );
      }
    ),
    "resolve_module() - resolve from source"
    >: (
      () => {
        let resolver = Resolver.create(__cache, __root_dir, __source_dir);

        Assert.module_(
          Resolve.Module.File({
            relative: Filename.concat(__source_dir, __path),
            full:
              String.join(
                ~separator=Filename.dir_sep,
                [__root_dir, __source_dir, __path],
              ),
          }),
          Resolver.resolve_module(~skip_cache=true, __id, resolver),
        );
      }
    ),
    "resolve_module() - resolve external"
    >: (
      () => {
        let resolver = Resolver.create(__cache, __root_dir, __source_dir);

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
