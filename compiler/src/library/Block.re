open Error;
open Infix;

/**
 container for working on fragments of an source document
 */
type t('a) = ('a, Cursor.t);

/* static */

let create = (value: 'a, cursor: Cursor.t): t('a) => (value, cursor);

/* getters */

let value = (block: t('a)): 'a => block |> fst;
let cursor = (block: t('a)): Cursor.t => block |> snd;
