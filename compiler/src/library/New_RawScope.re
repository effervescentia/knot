open Infix;
open Reference;

type t = NestedHashtbl.t(Identifier.t, New_RawType.t);

/* static */

let create = (): t => NestedHashtbl.create();

/* methods */

let child = (scope: t): t => NestedHashtbl.child(scope);

let define = (key: Identifier.t, raw_type: New_RawType.t, scope: t) =>
  scope |> NestedHashtbl.set(key, raw_type);
