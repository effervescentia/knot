use crate::{expression, matcher as m};
use combine::{
    attempt, choice, many, many1, none_of, optional, parser, parser::char as p, Parser, Stream,
};
use lang::ast;

fn fragment<T>() -> impl Parser<T, Output = ast::raw::Component>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::between(
        // avoid m::symbol to preserve trailing spaces
        m::span(m::sequence("<>")),
        // avoid m::symbol to preserve trailing spaces
        m::span(m::sequence("</>")),
        children().map(ast::Component::Fragment),
    )
    .map(|(value, range)| ast::raw::Component::raw(value, range))
}

fn children<T>() -> impl Parser<T, Output = Vec<ast::raw::Component>>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    enum Layout {
        Inline,
        Block,
    }

    many::<Vec<_>, _, _>(child()).map(|xs| {
        let layouts = xs
            .iter()
            .map(|x| match x.0.value() {
                ast::Component::Text(_) | ast::Component::Expression(_) => Layout::Inline,
                ast::Component::Fragment(_)
                | ast::Component::ClosedElement { .. }
                | ast::Component::OpenElement { .. } => Layout::Block,
            })
            .collect::<Vec<_>>();

        xs.into_iter()
            .enumerate()
            .map(|(i, x)| {
                ast::meta::Component(x.0.map_value(|x| match &x {
                    ast::Component::Text(s) => match (
                        if i == 0 { None } else { layouts.get(i - 1) },
                        layouts.get(i + 1),
                    ) {
                        (None | Some(Layout::Block), None | Some(Layout::Block)) => {
                            ast::Component::Text((*s).trim().to_owned())
                        }

                        (None | Some(Layout::Block), Some(Layout::Inline)) => {
                            ast::Component::Text((*s).trim_start().to_owned())
                        }

                        (Some(Layout::Inline), None | Some(Layout::Block)) => {
                            ast::Component::Text((*s).trim_end().to_owned())
                        }

                        (Some(Layout::Inline), Some(Layout::Inline)) => x,
                    },

                    _ => x,
                }))
            })
            .filter(|x| !matches!(x.0.value(), ast::Component::Text(s) if s.is_empty()))
            .collect()
    })
}

fn attribute<T>() -> impl Parser<T, Output = ast::raw::Attribute>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    (
        m::standard_identifier(),
        optional(m::symbol('=').with(expression::component_term())),
    )
        .map(|((name, start), value)| {
            if let Some(value) = value {
                let range = &start + value.0.range();

                ast::raw::Attribute::raw(ast::Attribute::Explicit(name, value), range)
            } else {
                ast::raw::Attribute::raw(ast::Attribute::Punned(name), start)
            }
        })
}

pub fn closed_element<T>() -> impl Parser<T, Output = ast::raw::Component>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    attempt(m::between(
        m::symbol('<'),
        m::span(m::sequence("/>")),
        (
            m::standard_identifier().map(|(x, _)| x),
            many::<Vec<_>, _, _>(attribute()),
        ),
    ))
    .map(|((name, attributes), range)| {
        ast::raw::Component::raw(ast::Component::ClosedElement(name, attributes), range)
    })
}

pub fn open_element<T>() -> impl Parser<T, Output = ast::raw::Component>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    (
        attempt(m::between(
            m::symbol('<'),
            // avoid m::symbol to preserve trailing spaces
            m::span(p::char('>')),
            (
                m::standard_identifier().map(|(x, _)| x),
                many::<Vec<_>, _, _>(attribute()),
            ),
        )),
        children(),
        m::between(
            m::glyph("</"),
            // avoid m::symbol to preserve trailing spaces
            m::span(p::char('>')),
            m::standard_identifier().map(|(x, _)| x),
        ),
    )
        .map(
            |(((start_tag, attributes), start), children, (end_tag, end))| {
                ast::raw::Component::raw(
                    ast::Component::open_element(start_tag, attributes, children, end_tag),
                    &start + &end,
                )
            },
        )
}

pub fn element<T>() -> impl Parser<T, Output = ast::raw::Component>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    choice((closed_element(), open_element()))
}

fn inline<T>() -> impl Parser<T, Output = ast::raw::Component>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::between(
        m::symbol('{'),
        // avoid m::symbol to preserve trailing spaces
        m::span(p::char('}')),
        expression::expression().map(ast::Component::Expression),
    )
    .map(|(value, range)| ast::raw::Component::raw(value, range))
}

fn text<T>() -> impl Parser<T, Output = ast::raw::Component>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::lexeme(many1(none_of(vec!['<', '{'])).map(ast::Component::Text))
        .map(|(value, range)| ast::raw::Component::raw(value, range))
}

parser! {
    fn child[T]()(T) -> ast::raw::Component
    where
        [T: Stream<Token = char>, T::Position: m::Position]
    {
        choice((component(), inline(), text()))
    }
}

parser! {
    pub fn component[T]()(T) -> ast::raw::Component
    where
        [T: Stream<Token = char>, T::Position: m::Position]
    {
        choice((fragment(), element()))
    }
}

#[cfg(test)]
mod tests {
    use super::component;
    use combine::{eof, stream::position::Stream, EasyParser, Parser};

    fn parse(s: &str) -> crate::Result<lang::ast::raw::Component> {
        component().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn fragment() {
        let ast = parse("<></>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn open_element() {
        let ast = parse("<foo></foo>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn closed_element() {
        let ast = parse("<foo />").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn fragment_in_fragment() {
        let ast = parse("<><></></>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn element_in_fragment() {
        let ast = parse("<><foo /></>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn fragment_in_element() {
        let ast = parse("<foo><></></foo>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn element_in_element() {
        let ast = parse("<foo><bar /></foo>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn inline_in_fragment() {
        let ast = parse("<>{nil}</>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn inline_in_element() {
        let ast = parse("<foo>{nil}</foo>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn text_in_fragment() {
        let ast = parse("<>foo</>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn text_in_element() {
        let ast = parse("<foo>bar</foo>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn attribute_on_element() {
        let ast = parse("<foo bar=nil></foo>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn attribute_on_self_closing_element() {
        let ast = parse("<foo bar=nil />").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn attribute_punned() {
        let ast = parse("<foo bar />").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn trim_text() {
        let ast = parse("<foo>  \n  \n  bar  \n  \n  </foo>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn trim_start_text() {
        let ast = parse("<foo>  \n  \n  bar  {fizz}\n</foo>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn trim_end_text() {
        let ast = parse("<foo>\n{fizz}  bar  \n  \n  </foo>").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn drop_empty_text() {
        let ast = parse(
            "<foo>
  <bar />
  <fizz />
</foo>",
        )
        .unwrap()
        .0;

        insta::assert_debug_snapshot!(ast);
    }
}
