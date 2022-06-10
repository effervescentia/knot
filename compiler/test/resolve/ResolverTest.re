open Kore;

module Resolver = Resolve.Resolver;

let __cache = "foo";
let __root_dir = "bar";
let __source_dir = "fizz";
let __path = "foo.kn";

let suite =
  "Resolve.Resolver"
  >::: [
    "create()"
    >: (
      () =>
        Assert.resolver(
          {cache: __cache, root_dir: __root_dir, source_dir: __source_dir},
          Resolver.create(__cache, __root_dir, __source_dir),
        )
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
          Resolver.resolve_module(Fixtures.Namespace.foo, resolver),
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
            full: Filename.join([__root_dir, __source_dir, __path]),
          }),
          Resolver.resolve_module(
            ~skip_cache=true,
            Fixtures.Namespace.foo,
            resolver,
          ),
        );
      }
    ),
    "resolve_module() - resolve external"
    >: (
      () => {
        let resolver = Resolver.create(__cache, __root_dir, __source_dir);

        Assert.throws(
          NotImplemented, "should throw NotImplemented exception", () =>
          Resolver.resolve_module(
            ~skip_cache=true,
            External(__path),
            resolver,
          )
          |> ignore
        );
      }
    ),
    "pp()"
    >: (
      () => {
        let resolver = Resolver.create(__cache, __root_dir, __source_dir);

        Assert.string(
          "Resolver {
  cache: foo
  root_dir: bar
}",
          resolver |> ~@Fmt.root(Resolver.pp),
        );
      }
    ),
  ];
