use crate::{
    ast::{ExpressionNode, KSXNode, KSX},
    common::position::Decrement,
    parser::{expression, matcher as m},
};
use combine::{
    attempt, choice, many, many1, none_of, optional, parser, parser::char as p, Parser, Stream,
};
use std::{fmt::Debug, vec};

fn fragment<T>() -> impl Parser<T, Output = KSXNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    m::between(
        // avoid m::symbol to preserve trailing spaces
        m::span(m::sequence("<>")),
        // avoid m::symbol to preserve trailing spaces
        m::span(m::sequence("</>")),
        children().map(KSX::Fragment),
    )
    .map(KSXNode::bind)
}

fn children<T>() -> impl Parser<T, Output = Vec<KSXNode<T, ()>>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    enum Layout {
        Inline,
        Block,
    }

    many::<Vec<_>, _, _>(child()).map(|xs| {
        let layouts = xs
            .iter()
            .map(|x| match x.node().value() {
                KSX::Text(_) | KSX::Inline(_) => Layout::Inline,
                KSX::Fragment(_) | KSX::ClosedElement(..) | KSX::OpenElement(..) => Layout::Block,
            })
            .collect::<Vec<_>>();

        xs.into_iter()
            .enumerate()
            .map(|(i, x)| {
                KSXNode(x.0.map_value(|x| match &x {
                    KSX::Text(s) => match (
                        if i == 0 { None } else { layouts.get(i - 1) },
                        layouts.get(i + 1),
                    ) {
                        (None | Some(Layout::Block), None | Some(Layout::Block)) => {
                            KSX::Text((*s).trim().to_string())
                        }

                        (None | Some(Layout::Block), Some(Layout::Inline)) => {
                            KSX::Text((*s).trim_start().to_string())
                        }

                        (Some(Layout::Inline), None | Some(Layout::Block)) => {
                            KSX::Text((*s).trim_end().to_string())
                        }

                        (Some(Layout::Inline), Some(Layout::Inline)) => x,
                    },

                    _ => x,
                }))
            })
            .filter(|x| match x.node().value() {
                KSX::Text(s) if s.is_empty() => false,
                _ => true,
            })
            .collect()
    })
}

fn attribute<T>() -> impl Parser<T, Output = (String, Option<ExpressionNode<T, ()>>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    (
        m::standard_identifier().map(|(x, _)| x),
        optional(m::symbol('=').with(expression::ksx_term())),
    )
}

pub fn closed_element<T>() -> impl Parser<T, Output = KSXNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
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
    .map(|((name, attributes), range)| KSXNode::raw(KSX::ClosedElement(name, attributes), range))
}

pub fn open_element<T>() -> impl Parser<T, Output = KSXNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
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
                KSXNode::raw(
                    KSX::OpenElement(start_tag, attributes, children, end_tag),
                    &start + &end,
                )
            },
        )
}

pub fn element<T>() -> impl Parser<T, Output = KSXNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    choice((closed_element(), open_element()))
}

fn inline<T>() -> impl Parser<T, Output = KSXNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    m::between(
        m::symbol('{'),
        // avoid m::symbol to preserve trailing spaces
        m::span(p::char('}')),
        expression::expression().map(KSX::Inline),
    )
    .map(KSXNode::bind)
}

fn text<T>() -> impl Parser<T, Output = KSXNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    m::lexeme(many1(none_of(vec!['<', '{'])).map(KSX::Text)).map(KSXNode::bind)
}

parser! {
    fn child[T]()(T) -> KSXNode<T, ()>
    where
        [T: Stream<Token = char>, T::Position: Copy + Debug + Decrement]
    {
        choice((ksx(), inline(), text()))
    }
}

parser! {
    pub fn ksx[T]()(T) -> KSXNode<T, ()>
    where
        [T: Stream<Token = char>, T::Position: Copy + Debug + Decrement]
    {
        choice((fragment(), element()))
    }
}

#[cfg(test)]
mod tests {
    use super::{ksx, KSXNode, KSX};
    use crate::{
        ast::{Expression, Primitive},
        parser::{CharStream, ParseResult},
        test::fixture as f,
    };
    use combine::{eof, stream::position::Stream, EasyParser, Parser};

    fn parse(s: &str) -> ParseResult<KSXNode<CharStream, ()>> {
        ksx().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn fragment() {
        assert_eq!(
            parse("<></>").unwrap().0,
            f::n::kxr(KSX::Fragment(vec![]), ((1, 1), (1, 5)))
        );
    }

    #[test]
    fn open_element() {
        assert_eq!(
            parse("<foo></foo>").unwrap().0,
            f::n::kxr(
                KSX::OpenElement(String::from("foo"), vec![], vec![], String::from("foo")),
                ((1, 1), (1, 11))
            )
        );
    }

    #[test]
    fn closed_element() {
        assert_eq!(
            parse("<foo />").unwrap().0,
            f::n::kxr(
                KSX::ClosedElement(String::from("foo"), vec![]),
                ((1, 1), (1, 7))
            )
        );
    }

    #[test]
    fn fragment_in_fragment() {
        assert_eq!(
            parse("<><></></>").unwrap().0,
            f::n::kxr(
                KSX::Fragment(vec![f::n::kxr(KSX::Fragment(vec![]), ((1, 3), (1, 7)))]),
                ((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn element_in_fragment() {
        assert_eq!(
            parse("<><foo /></>").unwrap().0,
            f::n::kxr(
                KSX::Fragment(vec![f::n::kxr(
                    KSX::ClosedElement(String::from("foo"), vec![]),
                    ((1, 3), (1, 9))
                )]),
                ((1, 1), (1, 12))
            )
        );
    }

    #[test]
    fn fragment_in_element() {
        assert_eq!(
            parse("<foo><></></foo>").unwrap().0,
            f::n::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![],
                    vec![f::n::kxr(KSX::Fragment(vec![]), ((1, 6), (1, 10)))],
                    String::from("foo"),
                ),
                ((1, 1), (1, 16))
            )
        );
    }

    #[test]
    fn element_in_element() {
        assert_eq!(
            parse("<foo><bar /></foo>").unwrap().0,
            f::n::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![],
                    vec![f::n::kxr(
                        KSX::ClosedElement(String::from("bar"), vec![]),
                        ((1, 6), (1, 12))
                    )],
                    String::from("foo"),
                ),
                ((1, 1), (1, 18))
            )
        );
    }

    #[test]
    fn inline_in_fragment() {
        assert_eq!(
            parse("<>{nil}</>").unwrap().0,
            f::n::kxr(
                KSX::Fragment(vec![f::n::kxr(
                    KSX::Inline(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 4), (1, 6))
                    )),
                    ((1, 3), (1, 7))
                )]),
                ((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn inline_in_element() {
        assert_eq!(
            parse("<foo>{nil}</foo>").unwrap().0,
            f::n::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![],
                    vec![f::n::kxr(
                        KSX::Inline(f::n::xr(
                            Expression::Primitive(Primitive::Nil),
                            ((1, 7), (1, 9))
                        )),
                        ((1, 6), (1, 10))
                    )],
                    String::from("foo"),
                ),
                ((1, 1), (1, 16))
            )
        );
    }

    #[test]
    fn text_in_fragment() {
        assert_eq!(
            parse("<>foo</>").unwrap().0,
            f::n::kxr(
                KSX::Fragment(vec![f::n::kxr(
                    KSX::Text(String::from("foo")),
                    ((1, 3), (1, 5))
                )]),
                ((1, 1), (1, 8))
            )
        );
    }

    #[test]
    fn text_in_element() {
        assert_eq!(
            parse("<foo>bar</foo>").unwrap().0,
            f::n::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![],
                    vec![f::n::kxr(KSX::Text(String::from("bar")), ((1, 6), (1, 8)))],
                    String::from("foo"),
                ),
                ((1, 1), (1, 14))
            )
        );
    }

    #[test]
    fn attribute_on_element() {
        assert_eq!(
            parse("<foo bar=nil></foo>").unwrap().0,
            f::n::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![(
                        String::from("bar"),
                        Some(f::n::xr(
                            Expression::Primitive(Primitive::Nil),
                            ((1, 10), (1, 12))
                        ))
                    )],
                    vec![],
                    String::from("foo"),
                ),
                ((1, 1), (1, 19))
            )
        );
    }

    #[test]
    fn attribute_on_self_closing_element() {
        assert_eq!(
            parse("<foo bar=nil />").unwrap().0,
            f::n::kxr(
                KSX::ClosedElement(
                    String::from("foo"),
                    vec![(
                        String::from("bar"),
                        Some(f::n::xr(
                            Expression::Primitive(Primitive::Nil),
                            ((1, 10), (1, 12))
                        ))
                    )],
                ),
                ((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn attribute_punned() {
        assert_eq!(
            parse("<foo bar />").unwrap().0,
            f::n::kxr(
                KSX::ClosedElement(String::from("foo"), vec![(String::from("bar"), None)],),
                ((1, 1), (1, 11))
            )
        );
    }

    #[test]
    fn trim_text() {
        assert_eq!(
            parse("<foo>  \n  \n  bar  \n  \n  </foo>").unwrap().0,
            f::n::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![],
                    vec![f::n::kxr(KSX::Text(String::from("bar")), ((1, 6), (5, 2)))],
                    String::from("foo"),
                ),
                ((1, 1), (5, 8))
            )
        );
    }

    #[test]
    fn trim_start_text() {
        assert_eq!(
            parse("<foo>  \n  \n  bar  {fizz}\n</foo>").unwrap().0,
            f::n::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![],
                    vec![
                        f::n::kxr(KSX::Text(String::from("bar  ")), ((1, 6), (3, 7))),
                        f::n::kxr(
                            KSX::Inline(f::n::xr(
                                Expression::Identifier(String::from("fizz")),
                                ((3, 9), (3, 12))
                            )),
                            ((3, 8), (3, 13))
                        )
                    ],
                    String::from("foo"),
                ),
                ((1, 1), (4, 6))
            )
        );
    }

    #[test]
    fn trim_end_text() {
        assert_eq!(
            parse("<foo>\n{fizz}  bar  \n  \n  </foo>").unwrap().0,
            f::n::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![],
                    vec![
                        f::n::kxr(
                            KSX::Inline(f::n::xr(
                                Expression::Identifier(String::from("fizz")),
                                ((2, 2), (2, 5))
                            )),
                            ((2, 1), (2, 6))
                        ),
                        f::n::kxr(KSX::Text(String::from("  bar")), ((2, 7), (4, 2)))
                    ],
                    String::from("foo"),
                ),
                ((1, 1), (4, 8))
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
            f::n::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![],
                    vec![
                        f::n::kxr(
                            KSX::ClosedElement(String::from("bar"), vec![]),
                            ((2, 3), (2, 9))
                        ),
                        f::n::kxr(
                            KSX::ClosedElement(String::from("fizz"), vec![]),
                            ((3, 3), (3, 10))
                        )
                    ],
                    String::from("foo"),
                ),
                ((1, 1), (4, 6))
            )
        );
    }
}
