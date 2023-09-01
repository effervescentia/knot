use crate::{
    expression::{self, ExpressionRaw},
    matcher as m,
    range::Range,
};
use combine::{attempt, between, choice, many, many1, none_of, optional, parser, Parser, Stream};
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub enum KSX<E, K> {
    Fragment(Vec<K>),
    Element(String, Vec<(String, Option<E>)>, Vec<K>),
    Inline(E),
    Text(String),
}

#[derive(Debug, PartialEq)]
pub struct KSXRaw<T>(pub KSX<ExpressionRaw<T>, KSXRaw<T>>, pub Range<T>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug;

impl<T> KSXRaw<T>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    pub fn bind((x, range): (KSX<ExpressionRaw<T>, KSXRaw<T>>, Range<T>)) -> Self {
        Self(x, range)
    }

    pub fn get_range(&self) -> &Range<T> {
        match self {
            KSXRaw(_, range) => range,
        }
    }
}

fn fragment<T>() -> impl Parser<T, Output = KSXRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    m::between(
        m::glyph("<>"),
        m::glyph("</>"),
        many::<Vec<_>, _, _>(child()).map(KSX::Fragment),
    )
    .map(KSXRaw::bind)
}

pub fn element<T>() -> impl Parser<T, Output = KSXRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    let attribute = (
        m::standard_identifier().map(|(x, _)| x),
        optional(m::symbol('=').with(expression::ksx_term())),
    );

    (
        attempt(m::symbol('<').with(m::standard_identifier())),
        many::<Vec<_>, _, _>(attribute),
        choice((
            m::glyph("/>").map(|_| Vec::new()),
            between(
                m::symbol('>'),
                (m::glyph("</"), m::standard_identifier(), m::symbol('>')),
                many(child()),
            ),
        )),
    )
        // TODO: combine with range from children
        .map(|((name, start), attributes, children)| {
            KSXRaw(KSX::Element(name, attributes, children), start)
        })
}

fn inline<T>() -> impl Parser<T, Output = KSXRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    m::between(
        m::symbol('{'),
        m::symbol('}'),
        expression::expression().map(KSX::Inline),
    )
    .map(KSXRaw::bind)
}

fn text<T>() -> impl Parser<T, Output = KSXRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    m::lexeme(many1(none_of(vec!['<', '{'])).map(KSX::Text)).map(KSXRaw::bind)
}

parser! {
    fn child[T]()(T) -> KSXRaw<T>
    where
        [T: Stream<Token = char>, T::Position: Copy + Debug]
    {
        choice((ksx(), inline(), text()))
    }
}

parser! {
    pub fn ksx[T]()(T) -> KSXRaw<T>
    where
        [T: Stream<Token = char>, T::Position: Copy + Debug]
    {
        choice((fragment(), element()))
    }
}

#[cfg(test)]
mod tests {
    use super::{ksx, KSXRaw, KSX};
    use crate::{
        expression::{primitive::Primitive, Expression, ExpressionRaw},
        range::Range,
    };
    use combine::Parser;

    #[test]
    fn fragment() {
        let parse = |s| ksx().parse(s);

        assert_eq!(
            parse("<></>").unwrap().0,
            KSXRaw(KSX::Fragment(vec![]), Range::str(1, 1))
        );
    }

    #[test]
    fn element() {
        let parse = |s| ksx().parse(s);

        assert_eq!(
            parse("<foo />").unwrap().0,
            KSXRaw(
                KSX::Element(String::from("foo"), vec![], vec![]),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("<foo></foo>").unwrap().0,
            KSXRaw(
                KSX::Element(String::from("foo"), vec![], vec![]),
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn nested() {
        let parse = |s| ksx().parse(s);

        assert_eq!(
            parse("<><></></>").unwrap().0,
            KSXRaw(
                KSX::Fragment(vec![KSXRaw(KSX::Fragment(vec![]), Range::str(1, 1))]),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("<><foo /></>").unwrap().0,
            KSXRaw(
                KSX::Fragment(vec![KSXRaw(
                    KSX::Element(String::from("foo"), vec![], vec![]),
                    Range::str(1, 1)
                )]),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("<foo><></></foo>").unwrap().0,
            KSXRaw(
                KSX::Element(
                    String::from("foo"),
                    vec![],
                    vec![KSXRaw(KSX::Fragment(vec![]), Range::str(1, 1))]
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("<foo><bar /></foo>").unwrap().0,
            KSXRaw(
                KSX::Element(
                    String::from("foo"),
                    vec![],
                    vec![KSXRaw(
                        KSX::Element(String::from("bar"), vec![], vec![]),
                        Range::str(1, 1)
                    )]
                ),
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn inline() {
        let parse = |s| ksx().parse(s);

        assert_eq!(
            parse("<>{nil}</>").unwrap().0,
            KSXRaw(
                KSX::Fragment(vec![KSXRaw(
                    KSX::Inline(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Range::str(1, 1)
                )]),
                Range::str(1, 1)
            )
        );
        let x = parse("<foo>{nil}</foo>").unwrap().0.get_range();
        assert_eq!(
            parse("<foo>{nil}</foo>").unwrap().0,
            KSXRaw(
                KSX::Element(
                    String::from("foo"),
                    vec![],
                    vec![KSXRaw(
                        KSX::Inline(ExpressionRaw(
                            Expression::Primitive(Primitive::Nil),
                            Range::str(1, 1)
                        )),
                        Range::str(1, 1)
                    )]
                ),
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn text() {
        let parse = |s| ksx().parse(s);

        assert_eq!(
            parse("<>foo</>").unwrap().0,
            KSXRaw(
                KSX::Fragment(vec![KSXRaw(
                    KSX::Text(String::from("foo")),
                    Range::str(1, 1)
                )]),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("<foo>bar</foo>").unwrap().0,
            KSXRaw(
                KSX::Element(
                    String::from("foo"),
                    vec![],
                    vec![KSXRaw(KSX::Text(String::from("bar")), Range::str(1, 1))]
                ),
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn attribute() {
        let parse = |s| ksx().parse(s);

        assert_eq!(
            parse("<foo bar />").unwrap().0,
            KSXRaw(
                KSX::Element(
                    String::from("foo"),
                    vec![(String::from("bar"), None)],
                    vec![]
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("<foo bar=nil />").unwrap().0,
            KSXRaw(
                KSX::Element(
                    String::from("foo"),
                    vec![(
                        String::from("bar"),
                        Some(ExpressionRaw(
                            Expression::Primitive(Primitive::Nil),
                            Range::str(1, 1)
                        ))
                    )],
                    vec![]
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("<foo bar=nil></foo>").unwrap().0,
            KSXRaw(
                KSX::Element(
                    String::from("foo"),
                    vec![(
                        String::from("bar"),
                        Some(ExpressionRaw(
                            Expression::Primitive(Primitive::Nil),
                            Range::str(1, 1)
                        ))
                    )],
                    vec![]
                ),
                Range::str(1, 1)
            )
        );
    }
}
