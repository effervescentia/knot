/**
 represents a hierarchy of scopes
 */
type t('a) = RangeTree2.t(Hashtbl.t(string, Type.t));

/* methods */

let rec of_scope = (scope: Scope.t): t('a) =>
  Node(
    Hashtbl.create(0),
    /* scope.types |> Hashtbl.map_values(Type.of_raw), */
    scope.range,
    scope.children |> List.map(of_scope),
  );
