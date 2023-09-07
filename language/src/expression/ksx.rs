use crate::{
    expression::{self, ExpressionRaw},
    matcher as m,
    position::Decrement,
    range::{Range, Ranged},
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
    T::Position: Copy + Debug + Decrement;

impl<T> KSXRaw<T>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn bind((x, range): (KSX<ExpressionRaw<T>, KSXRaw<T>>, Range<T>)) -> Self {
        Self(x, range)
    }
}

impl<T> Ranged<KSX<ExpressionRaw<T>, KSXRaw<T>>, T> for KSXRaw<T>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn value(self) -> KSX<ExpressionRaw<T>, KSXRaw<T>> {
        self.0
    }

    fn range(&self) -> &Range<T> {
        &self.1
    }
}

fn fragment<T>() -> impl Parser<T, Output = KSXRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
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
    T::Position: Copy + Debug + Decrement,
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
    T::Position: Copy + Debug + Decrement,
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
    T::Position: Copy + Debug + Decrement,
{
    m::lexeme(many1(none_of(vec!['<', '{'])).map(KSX::Text)).map(KSXRaw::bind)
}

parser! {
    fn child[T]()(T) -> KSXRaw<T>
    where
        [T: Stream<Token = char>, T::Position: Copy + Debug + Decrement]
    {
        choice((ksx(), inline(), text()))
    }
}

parser! {
    pub fn ksx[T]()(T) -> KSXRaw<T>
    where
        [T: Stream<Token = char>, T::Position: Copy + Debug + Decrement]
    {
        choice((fragment(), element()))
    }
}

#[cfg(test)]
mod tests {
    use super::{ksx, KSXRaw, KSX};
    use crate::{
        expression::{primitive::Primitive, Expression},
        test::fixture as f,
        CharStream, ParseResult,
    };
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> ParseResult<KSXRaw<CharStream>> {
        ksx().easy_parse(Stream::new(s))
    }

    #[test]
    fn fragment() {
        assert_eq!(
            parse("<></>").unwrap().0,
            f::kxr(KSX::Fragment(vec![]), ((1, 1), (1, 5)))
        );
    }

    #[test]
    fn element() {
        assert_eq!(
            parse("<foo></foo>").unwrap().0,
            f::kxr(
                KSX::Element(String::from("foo"), vec![], vec![]),
                ((1, 1), (1, 11))
            )
        );
    }

    #[test]
    fn element_self_closing() {
        assert_eq!(
            parse("<foo />").unwrap().0,
            f::kxr(
                KSX::Element(String::from("foo"), vec![], vec![]),
                ((1, 1), (1, 7))
            )
        );
    }

    #[test]
    fn fragment_in_fragment() {
        assert_eq!(
            parse("<><></></>").unwrap().0,
            f::kxr(
                KSX::Fragment(vec![f::kxr(KSX::Fragment(vec![]), ((1, 1), (1, 1)))]),
                ((1, 1), (1, 1))
            )
        );
    }

    #[test]
    fn element_in_fragment() {
        assert_eq!(
            parse("<><foo /></>").unwrap().0,
            f::kxr(
                KSX::Fragment(vec![f::kxr(
                    KSX::Element(String::from("foo"), vec![], vec![]),
                    ((1, 1), (1, 1))
                )]),
                ((1, 1), (1, 1))
            )
        );
    }

    #[test]
    fn fragment_in_element() {
        assert_eq!(
            parse("<foo><></></foo>").unwrap().0,
            f::kxr(
                KSX::Element(
                    String::from("foo"),
                    vec![],
                    vec![f::kxr(KSX::Fragment(vec![]), ((1, 1), (1, 1)))]
                ),
                ((1, 1), (1, 1))
            )
        );
    }

    #[test]
    fn element_in_element() {
        assert_eq!(
            parse("<foo><bar /></foo>").unwrap().0,
            f::kxr(
                KSX::Element(
                    String::from("foo"),
                    vec![],
                    vec![f::kxr(
                        KSX::Element(String::from("bar"), vec![], vec![]),
                        ((1, 1), (1, 1))
                    )]
                ),
                ((1, 1), (1, 1))
            )
        );
    }

    #[test]
    fn inline_in_fragment() {
        assert_eq!(
            parse("<>{nil}</>").unwrap().0,
            f::kxr(
                KSX::Fragment(vec![f::kxr(
                    KSX::Inline(f::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 1))
                    )),
                    ((1, 1), (1, 1))
                )]),
                ((1, 1), (1, 1))
            )
        );
    }

    #[test]
    fn inline_in_element() {
        assert_eq!(
            parse("<foo>{nil}</foo>").unwrap().0,
            f::kxr(
                KSX::Element(
                    String::from("foo"),
                    vec![],
                    vec![f::kxr(
                        KSX::Inline(f::xr(
                            Expression::Primitive(Primitive::Nil),
                            ((1, 1), (1, 1))
                        )),
                        ((1, 1), (1, 1))
                    )]
                ),
                ((1, 1), (1, 1))
            )
        );
    }

    #[test]
    fn text_in_fragment() {
        assert_eq!(
            parse("<>foo</>").unwrap().0,
            f::kxr(
                KSX::Fragment(vec![f::kxr(
                    KSX::Text(String::from("foo")),
                    ((1, 1), (1, 1))
                )]),
                ((1, 1), (1, 1))
            )
        );
    }

    #[test]
    fn text_in_element() {
        assert_eq!(
            parse("<foo>bar</foo>").unwrap().0,
            f::kxr(
                KSX::Element(
                    String::from("foo"),
                    vec![],
                    vec![f::kxr(KSX::Text(String::from("bar")), ((1, 1), (1, 1)))]
                ),
                ((1, 1), (1, 1))
            )
        );
    }

    #[test]
    fn attribute_on_element() {
        assert_eq!(
            parse("<foo bar=nil></foo>").unwrap().0,
            f::kxr(
                KSX::Element(
                    String::from("foo"),
                    vec![(
                        String::from("bar"),
                        Some(f::xr(
                            Expression::Primitive(Primitive::Nil),
                            ((1, 1), (1, 1))
                        ))
                    )],
                    vec![]
                ),
                ((1, 1), (1, 1))
            )
        );
    }

    #[test]
    fn attribute_on_self_closing_element() {
        assert_eq!(
            parse("<foo bar=nil />").unwrap().0,
            f::kxr(
                KSX::Element(
                    String::from("foo"),
                    vec![(
                        String::from("bar"),
                        Some(f::xr(
                            Expression::Primitive(Primitive::Nil),
                            ((1, 1), (1, 1))
                        ))
                    )],
                    vec![]
                ),
                ((1, 1), (1, 1))
            )
        );
    }

    #[test]
    fn attribute_punned() {
        assert_eq!(
            parse("<foo bar />").unwrap().0,
            f::kxr(
                KSX::Element(
                    String::from("foo"),
                    vec![(String::from("bar"), None)],
                    vec![]
                ),
                ((1, 1), (1, 1))
            )
        );
    }
}
