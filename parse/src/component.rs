use crate::{ast, expression, matcher as m};
use combine::{
    attempt, choice, many, many1, none_of, optional, parser, parser::char as p, Parser, Stream,
};

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
    .map(|(value, range)| ast::raw::Component::new(value, range))
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
                ast::raw::Component(x.0.map_value(|x| match &x {
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

fn attribute<T>() -> impl Parser<T, Output = (String, Option<ast::raw::Expression>)>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    (
        m::standard_identifier().map(|(x, _)| x),
        optional(m::symbol('=').with(expression::component_term())),
    )
}

pub fn closed_element<T>() -> impl Parser<T, Output = ast::raw::Component>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    attempt(m::between(
        m::symbol('<'),
        // avoid m::symbol to preserve trailing spaces
        m::span(m::sequence("/>")),
        (
            m::standard_identifier().map(|(x, _)| x),
            many::<Vec<_>, _, _>(attribute()),
        ),
    ))
    .map(|((name, attributes), range)| {
        ast::raw::Component::new(ast::Component::ClosedElement(name, attributes), range)
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
                ast::raw::Component::new(
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
    .map(|(value, range)| ast::raw::Component::new(value, range))
}

fn text<T>() -> impl Parser<T, Output = ast::raw::Component>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::lexeme(many1(none_of(vec!['<', '{'])).map(ast::Component::Text))
        .map(|(value, range)| ast::raw::Component::new(value, range))
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
    use crate::ast;
    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use kore::str;
    use lang::Range;

    fn parse(s: &str) -> crate::Result<ast::raw::Component> {
        component().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn fragment() {
        assert_eq!(
            parse("<></>").unwrap().0,
            ast::raw::Component::new(ast::Component::Fragment(vec![]), Range::new((1, 1), (1, 5)))
        );
    }

    #[test]
    fn open_element() {
        assert_eq!(
            parse("<foo></foo>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::open_element(str!("foo"), vec![], vec![], str!("foo")),
                Range::new((1, 1), (1, 11))
            )
        );
    }

    #[test]
    fn closed_element() {
        assert_eq!(
            parse("<foo />").unwrap().0,
            ast::raw::Component::new(
                ast::Component::ClosedElement(str!("foo"), vec![]),
                Range::new((1, 1), (1, 7))
            )
        );
    }

    #[test]
    fn fragment_in_fragment() {
        assert_eq!(
            parse("<><></></>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::Fragment(vec![ast::raw::Component::new(
                    ast::Component::Fragment(vec![]),
                    Range::new((1, 3), (1, 7))
                )]),
                Range::new((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn element_in_fragment() {
        assert_eq!(
            parse("<><foo /></>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::Fragment(vec![ast::raw::Component::new(
                    ast::Component::ClosedElement(str!("foo"), vec![]),
                    Range::new((1, 3), (1, 9))
                )]),
                Range::new((1, 1), (1, 12))
            )
        );
    }

    #[test]
    fn fragment_in_element() {
        assert_eq!(
            parse("<foo><></></foo>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::open_element(
                    str!("foo"),
                    vec![],
                    vec![ast::raw::Component::new(
                        ast::Component::Fragment(vec![]),
                        Range::new((1, 6), (1, 10))
                    )],
                    str!("foo"),
                ),
                Range::new((1, 1), (1, 16))
            )
        );
    }

    #[test]
    fn element_in_element() {
        assert_eq!(
            parse("<foo><bar /></foo>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::open_element(
                    str!("foo"),
                    vec![],
                    vec![ast::raw::Component::new(
                        ast::Component::ClosedElement(str!("bar"), vec![]),
                        Range::new((1, 6), (1, 12))
                    )],
                    str!("foo"),
                ),
                Range::new((1, 1), (1, 18))
            )
        );
    }

    #[test]
    fn inline_in_fragment() {
        assert_eq!(
            parse("<>{nil}</>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::Fragment(vec![ast::raw::Component::new(
                    ast::Component::Expression(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 4), (1, 6))
                    )),
                    Range::new((1, 3), (1, 7))
                )]),
                Range::new((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn inline_in_element() {
        assert_eq!(
            parse("<foo>{nil}</foo>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::open_element(
                    str!("foo"),
                    vec![],
                    vec![ast::raw::Component::new(
                        ast::Component::Expression(ast::raw::Expression::new(
                            ast::Expression::Primitive(ast::Primitive::Nil),
                            Range::new((1, 7), (1, 9))
                        )),
                        Range::new((1, 6), (1, 10))
                    )],
                    str!("foo"),
                ),
                Range::new((1, 1), (1, 16))
            )
        );
    }

    #[test]
    fn text_in_fragment() {
        assert_eq!(
            parse("<>foo</>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::Fragment(vec![ast::raw::Component::new(
                    ast::Component::Text(str!("foo")),
                    Range::new((1, 3), (1, 5))
                )]),
                Range::new((1, 1), (1, 8))
            )
        );
    }

    #[test]
    fn text_in_element() {
        assert_eq!(
            parse("<foo>bar</foo>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::open_element(
                    str!("foo"),
                    vec![],
                    vec![ast::raw::Component::new(
                        ast::Component::Text(str!("bar")),
                        Range::new((1, 6), (1, 8))
                    )],
                    str!("foo"),
                ),
                Range::new((1, 1), (1, 14))
            )
        );
    }

    #[test]
    fn attribute_on_element() {
        assert_eq!(
            parse("<foo bar=nil></foo>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::open_element(
                    str!("foo"),
                    vec![(
                        str!("bar"),
                        Some(ast::raw::Expression::new(
                            ast::Expression::Primitive(ast::Primitive::Nil),
                            Range::new((1, 10), (1, 12))
                        ))
                    )],
                    vec![],
                    str!("foo"),
                ),
                Range::new((1, 1), (1, 19))
            )
        );
    }

    #[test]
    fn attribute_on_self_closing_element() {
        assert_eq!(
            parse("<foo bar=nil />").unwrap().0,
            ast::raw::Component::new(
                ast::Component::ClosedElement(
                    str!("foo"),
                    vec![(
                        str!("bar"),
                        Some(ast::raw::Expression::new(
                            ast::Expression::Primitive(ast::Primitive::Nil),
                            Range::new((1, 10), (1, 12))
                        ))
                    )],
                ),
                Range::new((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn attribute_punned() {
        assert_eq!(
            parse("<foo bar />").unwrap().0,
            ast::raw::Component::new(
                ast::Component::ClosedElement(str!("foo"), vec![(str!("bar"), None)],),
                Range::new((1, 1), (1, 11))
            )
        );
    }

    #[test]
    fn trim_text() {
        assert_eq!(
            parse("<foo>  \n  \n  bar  \n  \n  </foo>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::open_element(
                    str!("foo"),
                    vec![],
                    vec![ast::raw::Component::new(
                        ast::Component::Text(str!("bar")),
                        Range::new((1, 6), (5, 2))
                    )],
                    str!("foo"),
                ),
                Range::new((1, 1), (5, 8))
            )
        );
    }

    #[test]
    fn trim_start_text() {
        assert_eq!(
            parse("<foo>  \n  \n  bar  {fizz}\n</foo>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::open_element(
                    str!("foo"),
                    vec![],
                    vec![
                        ast::raw::Component::new(
                            ast::Component::Text(str!("bar  ")),
                            Range::new((1, 6), (3, 7))
                        ),
                        ast::raw::Component::new(
                            ast::Component::Expression(ast::raw::Expression::new(
                                ast::Expression::Identifier(str!("fizz")),
                                Range::new((3, 9), (3, 12))
                            )),
                            Range::new((3, 8), (3, 13))
                        )
                    ],
                    str!("foo"),
                ),
                Range::new((1, 1), (4, 6))
            )
        );
    }

    #[test]
    fn trim_end_text() {
        assert_eq!(
            parse("<foo>\n{fizz}  bar  \n  \n  </foo>").unwrap().0,
            ast::raw::Component::new(
                ast::Component::open_element(
                    str!("foo"),
                    vec![],
                    vec![
                        ast::raw::Component::new(
                            ast::Component::Expression(ast::raw::Expression::new(
                                ast::Expression::Identifier(str!("fizz")),
                                Range::new((2, 2), (2, 5))
                            )),
                            Range::new((2, 1), (2, 6))
                        ),
                        ast::raw::Component::new(
                            ast::Component::Text(str!("  bar")),
                            Range::new((2, 7), (4, 2))
                        )
                    ],
                    str!("foo"),
                ),
                Range::new((1, 1), (4, 8))
            )
        );
    }

    #[test]
    fn drop_empty_text() {
        assert_eq!(
            parse(
                "<foo>
  <bar />
  <fizz />
</foo>"
            )
            .unwrap()
            .0,
            ast::raw::Component::new(
                ast::Component::open_element(
                    str!("foo"),
                    vec![],
                    vec![
                        ast::raw::Component::new(
                            ast::Component::ClosedElement(str!("bar"), vec![]),
                            Range::new((2, 3), (2, 9))
                        ),
                        ast::raw::Component::new(
                            ast::Component::ClosedElement(str!("fizz"), vec![]),
                            Range::new((3, 3), (3, 10))
                        )
                    ],
                    str!("foo"),
                ),
                Range::new((1, 1), (4, 6))
            )
        );
    }
}
