open Kore;
open Whitespace;
open Container;

type xml_attr_t('a) = (string, 'a);

type xml_t('a) =
  | Node(string, list(xml_attr_t('a)), list(xml_t('a)));

let node_attr = (pp_value: Fmt.t('a)): Fmt.t(xml_attr_t('a)) =>
  (ppf, (key, value)) => pf(ppf, "%s=%a", key, pp_value, value);

let _inline_node_attr_sep = Sep.(of_sep(~trail=Trail.space, ""));
let _node_attr_sep = Sep.(of_sep(~trail=Trail.newline, ""));

let rec xml = (pp_attr: Fmt.t('a)): Fmt.t(xml_t('a)) =>
  ppf =>
    fun
    | Node(name, attrs, []) =>
      pf(
        ppf,
        "@[<hv><%s%t%a/>@]",
        name,
        space_or_indent,
        list(~sep=_inline_node_attr_sep, node_attr(pp_attr)),
        attrs,
      )

    | Node(name, [], tags) =>
      pf(
        ppf,
        "@[<v><%s>%a</%s>@]",
        name,
        block(~layout=Vertical, ~sep=Sep.trailing_newline, xml(pp_attr)),
        tags,
        name,
      )

    | Node(name, attrs, tags) =>
      pf(
        ppf,
        "@[<v>@[<hv><%s%t%a>@]%a</%s>@]",
        name,
        space_or_indent,
        list(~sep=_node_attr_sep, node_attr(pp_attr)),
        attrs,
        block(~layout=Vertical, ~sep=Sep.trailing_newline, xml(pp_attr)),
        tags,
        name,
      );

let xml_string = xml(string);
