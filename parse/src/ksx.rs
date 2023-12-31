use crate::{expression, matcher as m, Position, Range};
use combine::{
    attempt, choice, many, many1, none_of, optional, parser, parser::char as p, Parser, Stream,
};
use lang::ast::{AstNode, ExpressionNode, KSXNode, KSX};

fn fragment<T>() -> impl Parser<T, Output = KSXNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    m::between(
        // avoid m::symbol to preserve trailing spaces
        m::span(m::sequence("<>")),
        // avoid m::symbol to preserve trailing spaces
        m::span(m::sequence("</>")),
        children().map(KSX::Fragment),
    )
    .map(|(value, range)| KSXNode::raw(value, range))
}

fn children<T>() -> impl Parser<T, Output = Vec<KSXNode<Range, ()>>>
where
    T: Stream<Token = char>,
    T::Position: Position,
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
                            KSX::Text((*s).trim().to_owned())
                        }

                        (None | Some(Layout::Block), Some(Layout::Inline)) => {
                            KSX::Text((*s).trim_start().to_owned())
                        }

                        (Some(Layout::Inline), None | Some(Layout::Block)) => {
                            KSX::Text((*s).trim_end().to_owned())
                        }

                        (Some(Layout::Inline), Some(Layout::Inline)) => x,
                    },

                    _ => x,
                }))
            })
            .filter(|x| !matches!(x.node().value(), KSX::Text(s) if s.is_empty()))
            .collect()
    })
}

fn attribute<T>() -> impl Parser<T, Output = (String, Option<ExpressionNode<Range, ()>>)>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    (
        m::standard_identifier().map(|(x, _)| x),
        optional(m::symbol('=').with(expression::ksx_term())),
    )
}

pub fn closed_element<T>() -> impl Parser<T, Output = KSXNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
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

pub fn open_element<T>() -> impl Parser<T, Output = KSXNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
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

pub fn element<T>() -> impl Parser<T, Output = KSXNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    choice((closed_element(), open_element()))
}

fn inline<T>() -> impl Parser<T, Output = KSXNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    m::between(
        m::symbol('{'),
        // avoid m::symbol to preserve trailing spaces
        m::span(p::char('}')),
        expression::expression().map(KSX::Inline),
    )
    .map(|(value, range)| KSXNode::raw(value, range))
}

fn text<T>() -> impl Parser<T, Output = KSXNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    m::lexeme(many1(none_of(vec!['<', '{'])).map(KSX::Text))
        .map(|(value, range)| KSXNode::raw(value, range))
}

parser! {
    fn child[T]()(T) -> KSXNode<Range, ()>
    where
        [T: Stream<Token = char>, T::Position: Position]
    {
        choice((ksx(), inline(), text()))
    }
}

parser! {
    pub fn ksx[T]()(T) -> KSXNode<Range, ()>
    where
        [T: Stream<Token = char>, T::Position: Position]
    {
        choice((fragment(), element()))
    }
}

#[cfg(test)]
mod tests {
    use super::{ksx, KSXNode, KSX};
    use crate::{test::fixture as f, Range};
    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use kore::str;
    use lang::ast::{Expression, Primitive};

    fn parse(s: &str) -> crate::Result<KSXNode<Range, ()>> {
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
                KSX::OpenElement(str!("foo"), vec![], vec![], str!("foo")),
                ((1, 1), (1, 11))
            )
        );
    }

    #[test]
    fn closed_element() {
        assert_eq!(
            parse("<foo />").unwrap().0,
            f::n::kxr(KSX::ClosedElement(str!("foo"), vec![]), ((1, 1), (1, 7)))
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
                    KSX::ClosedElement(str!("foo"), vec![]),
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
                    str!("foo"),
                    vec![],
                    vec![f::n::kxr(KSX::Fragment(vec![]), ((1, 6), (1, 10)))],
                    str!("foo"),
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
                    str!("foo"),
                    vec![],
                    vec![f::n::kxr(
                        KSX::ClosedElement(str!("bar"), vec![]),
                        ((1, 6), (1, 12))
                    )],
                    str!("foo"),
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
                    str!("foo"),
                    vec![],
                    vec![f::n::kxr(
                        KSX::Inline(f::n::xr(
                            Expression::Primitive(Primitive::Nil),
                            ((1, 7), (1, 9))
                        )),
                        ((1, 6), (1, 10))
                    )],
                    str!("foo"),
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
                KSX::Fragment(vec![f::n::kxr(KSX::Text(str!("foo")), ((1, 3), (1, 5)))]),
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
                    str!("foo"),
                    vec![],
                    vec![f::n::kxr(KSX::Text(str!("bar")), ((1, 6), (1, 8)))],
                    str!("foo"),
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
                    str!("foo"),
                    vec![(
                        str!("bar"),
                        Some(f::n::xr(
                            Expression::Primitive(Primitive::Nil),
                            ((1, 10), (1, 12))
                        ))
                    )],
                    vec![],
                    str!("foo"),
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
                    str!("foo"),
                    vec![(
                        str!("bar"),
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
                KSX::ClosedElement(str!("foo"), vec![(str!("bar"), None)],),
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
                    str!("foo"),
                    vec![],
                    vec![f::n::kxr(KSX::Text(str!("bar")), ((1, 6), (5, 2)))],
                    str!("foo"),
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
                    str!("foo"),
                    vec![],
                    vec![
                        f::n::kxr(KSX::Text(str!("bar  ")), ((1, 6), (3, 7))),
                        f::n::kxr(
                            KSX::Inline(f::n::xr(
                                Expression::Identifier(str!("fizz")),
                                ((3, 9), (3, 12))
                            )),
                            ((3, 8), (3, 13))
                        )
                    ],
                    str!("foo"),
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
                    str!("foo"),
                    vec![],
                    vec![
                        f::n::kxr(
                            KSX::Inline(f::n::xr(
                                Expression::Identifier(str!("fizz")),
                                ((2, 2), (2, 5))
                            )),
                            ((2, 1), (2, 6))
                        ),
                        f::n::kxr(KSX::Text(str!("  bar")), ((2, 7), (4, 2)))
                    ],
                    str!("foo"),
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
                    str!("foo"),
                    vec![],
                    vec![
                        f::n::kxr(KSX::ClosedElement(str!("bar"), vec![]), ((2, 3), (2, 9))),
                        f::n::kxr(KSX::ClosedElement(str!("fizz"), vec![]), ((3, 3), (3, 10)))
                    ],
                    str!("foo"),
                ),
                ((1, 1), (4, 6))
            )
        );
    }
}
