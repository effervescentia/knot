open Kore;

let __indent_spaces = 2;

type layout_t =
  | Vertical
  | Horizontal;

/* layout formatters */

let root = (pp_value: t('a)): t('a) => vbox(pp_value);

let page = (pp_value: t('a)): t('a) =>
  ppf => pf(ppf, "%a@.", root(pp_value));

/**
 print an indented value
 */
let indented = (pp_value: t('a)): t('a) =>
  ppf => pf(ppf, "%t%a", Whitespace.indent, pp_value);

let attribute = (pp_key: t('a), pp_value: t('b)): t(('a, 'b)) =>
  (ppf, (key, value)) => pf(ppf, "%a: %a", pp_key, key, pp_value, value);

/* container formatters */

let _list_box =
  fun
  | Vertical => vbox
  | Horizontal => hvbox;

/**
   print a list of values divided by separators and spaces
   automatically breaks each item onto separate lines for long lists
   set `~trail=true` to show a trailing separator for long lists
   */
let list =
    (~layout=Horizontal, ~sep=Sep.trailing_comma, pp_value: t('a))
    : t(list('a)) =>
  ppf =>
    fun
    /* do not add surrounding box when empty */
    | [] => nop(ppf, ())

    | xs => {
        let rec print = ppf => (
          fun
          | [] => nop(ppf, ())

          | [x] => pp_value(ppf, x)

          | [x, ...xs] =>
            pf(ppf, "%a%a%a", pp_value, x, sep.hint, (), print, xs)
        );

        pf(ppf, "%a%t", _list_box(layout, print), xs, sep.trail);
      };

/**
 print an indented list of values divided by separators
 */
let block =
    (~layout=Horizontal, ~sep=Sep.trailing_comma, pp_value: t('a))
    : t(list('a)) =>
  ppf =>
    fun
    | [] => nop(ppf, ())

    | xs => indented(list(~layout, ~sep, pp_value), ppf, xs);

/**
 print a list of values divided by separators
 and book-ended by open and close symbols
 */
let collection =
    (
      ~layout=Horizontal,
      ~sep=Sep.trailing_comma,
      pp_open: t(unit),
      pp_close: t(unit),
      pp_value: t('a),
    )
    : t(list('a)) =>
  (ppf, xs) =>
    pf(
      ppf,
      "%a%a%a",
      pp_open,
      (),
      block(~layout, ~sep, pp_value),
      xs,
      pp_close,
      (),
    );

let array = (pp_value: t('a)): t(list('a)) =>
  box(collection(any("["), any("]"), pp_value));

let tuple = (pp_value: t('a)): t(list('a)) =>
  box(collection(any("("), any(")"), pp_value));

let closure = (pp_value: t('a)): t(list('a)) =>
  ppf =>
    fun
    /* do not allow this space to be broken */
    | [] => string(ppf, "{ }")

    | xs =>
      collection(
        ~layout=Vertical,
        ~sep=Sep.trailing_newline,
        any("{"),
        any("}"),
        pp_value,
        ppf,
        xs,
      );

let record = (pp_key: t('a), pp_value: t('b)): t(list(('a, 'b))) =>
  closure(attribute(pp_key, pp_value));

let entity = (pp_sig: t('a), pp_entry: t('b)): t(('a, list('b))) =>
  (ppf, (sig_, entries)) =>
    pf(ppf, "@[<hv>%a@ @]%a", pp_sig, sig_, closure(pp_entry), entries);

let struct_ =
    (pp_key: t('a), pp_value: t('b)): t((string, list(('a, 'b)))) =>
  (ppf, (name, attrs)) =>
    entity(
      Color.bold(string),
      attribute(Color.bold(pp_key), pp_value),
      ppf,
      (name, attrs),
    );

let _destruct_comma_sep = Sep.(of_sep(~trail=Trail.space_or_comma, ","));

let destruct = (pp_entry, ppf, entries) =>
  pf(
    ppf,
    "{%t%a}",
    Whitespace.space_or_indent,
    list(~sep=_destruct_comma_sep, pp_entry),
    entries,
  );
