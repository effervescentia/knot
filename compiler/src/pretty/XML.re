open Kore;
open Whitespace;
open Container;

type tag_attr_t = (string, string);

type tag_t =
  | Parent(string, list(tag_attr_t), list(tag_t))
  | Child(string, list(tag_attr_t));

let tag_attr: Fmt.t(tag_attr_t) =
  (ppf, (key, value)) => pf(ppf, "%s=%s", key, value);

let _inline_tag_attr_sep = Sep.of_sep(~trail=ppf => sp(ppf, ()), "");
let _tag_attr_sep = Sep.of_sep(~trail=ppf => cut(ppf, ()), "");

let rec xml: Fmt.t(tag_t) =
  ppf =>
    fun
    | Parent(name, attrs, [])
    | Child(name, attrs) =>
      pf(
        ppf,
        "@[<hv><%s%t%a/>@]",
        name,
        space_or_indent,
        list(~sep=_inline_tag_attr_sep, tag_attr),
        attrs,
      )

    | Parent(name, [], tags) =>
      pf(
        ppf,
        "@[<v><%s>%a</%s>@]",
        name,
        block(~sep=Sep.trailing_newline, xml),
        tags,
        name,
      )

    | Parent(name, attrs, tags) =>
      pf(
        ppf,
        "@[<v>@[<hv><%s%t%a>@]%a</%s>@]",
        name,
        space_or_indent,
        list(~sep=_tag_attr_sep, tag_attr),
        attrs,
        block(~sep=Sep.trailing_newline, xml),
        tags,
        name,
      );
