open Knot.Core;
open Exception;

exception TypeMatchFailed;

/*
 what can you do with a reference?

 1. access a property
 2. operate on it
 3. call it...

 that's it?
 */

let enforces_key = key =>
  fun
  | HasProperty(name, _) when name == key => true
  | _ => false;
let enforces_callable =
  fun
  | HasCallSignature(_) => true
  | _ => false;

let is_callable = List.exists(enforces_callable);
let allows_callable = x => x == [];
let has_key = (rules, key) => List.exists(enforces_key(key), rules);
let allows_key = (rules, key) =>
  List.for_all(
    fun
    | HasProperty(_)
    | _ => false,
    rules,
  );
let get_prop_type = (rules, key) =>
  List.find(enforces_key(key), rules)
  |> (
    fun
    | HasProperty(_, typ) => typ
    | _ => raise(PropLookupFailed)
  );
let get_return_type = rules =>
  List.find(enforces_callable, rules)
  |> (
    fun
    | HasCallSignature(_, typ) => typ
    | _ => raise(PropLookupFailed)
  );

let allows_type = rules =>
  fun
  | Number_t => rules == []
  | String_t => rules == []
  | Boolean_t => rules == []
  | _ => raise(TypeMatchFailed);
