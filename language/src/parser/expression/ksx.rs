use crate::parser::{
    expression::{self, ExpressionRaw},
    matcher as m,
    position::Decrement,
    range::{Range, Ranged},
};
use combine::{attempt, choice, many, many1, none_of, optional, parser, Parser, Stream};
use std::{fmt::Debug, vec};

#[derive(Debug, PartialEq)]
pub enum KSX<E, K> {
    Fragment(Vec<K>),
    OpenElement(String, Vec<(String, Option<E>)>, Vec<K>, String),
    ClosedElement(String, Vec<(String, Option<E>)>),
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

fn attribute<T>() -> impl Parser<T, Output = (String, Option<ExpressionRaw<T>>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    (
        m::standard_identifier().map(|(x, _)| x),
        optional(m::symbol('=').with(expression::ksx_term())),
    )
}

pub fn closed_element<T>() -> impl Parser<T, Output = KSXRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    attempt(m::between(
        m::symbol('<'),
        m::glyph("/>"),
        (
            m::standard_identifier().map(|(x, _)| x),
            many::<Vec<_>, _, _>(attribute()),
        ),
    ))
    .map(|((name, attributes), range)| KSXRaw(KSX::ClosedElement(name, attributes), range))
}

pub fn open_element<T>() -> impl Parser<T, Output = KSXRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    (
        attempt(m::between(
            m::symbol('<'),
            m::symbol('>'),
            (
                m::standard_identifier().map(|(x, _)| x),
                many::<Vec<_>, _, _>(attribute()),
            ),
        )),
        many(child()),
        m::between(
            m::glyph("</"),
            m::symbol('>'),
            m::standard_identifier().map(|(x, _)| x),
        ),
    )
        .map(
            |(((start_tag, attributes), start), children, (end_tag, end))| {
                KSXRaw(
                    KSX::OpenElement(start_tag, attributes, children, end_tag),
                    &start + &end,
                )
            },
        )
}

pub fn element<T>() -> impl Parser<T, Output = KSXRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    choice((closed_element(), open_element()))
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
        parser::{
            expression::{primitive::Primitive, Expression},
            CharStream, ParseResult,
        },
        test::fixture as f,
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
    fn open_element() {
        assert_eq!(
            parse("<foo></foo>").unwrap().0,
            f::kxr(
                KSX::OpenElement(String::from("foo"), vec![], vec![], String::from("foo")),
                ((1, 1), (1, 11))
            )
        );
    }

    #[test]
    fn closed_element() {
        assert_eq!(
            parse("<foo />").unwrap().0,
            f::kxr(
                KSX::ClosedElement(String::from("foo"), vec![]),
                ((1, 1), (1, 7))
            )
        );
    }

    #[test]
    fn fragment_in_fragment() {
        assert_eq!(
            parse("<><></></>").unwrap().0,
            f::kxr(
                KSX::Fragment(vec![f::kxr(KSX::Fragment(vec![]), ((1, 3), (1, 7)))]),
                ((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn element_in_fragment() {
        assert_eq!(
            parse("<><foo /></>").unwrap().0,
            f::kxr(
                KSX::Fragment(vec![f::kxr(
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
            f::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![],
                    vec![f::kxr(KSX::Fragment(vec![]), ((1, 6), (1, 10)))],
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
            f::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![],
                    vec![f::kxr(
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
            f::kxr(
                KSX::Fragment(vec![f::kxr(
                    KSX::Inline(f::xr(
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
            f::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![],
                    vec![f::kxr(
                        KSX::Inline(f::xr(
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
            f::kxr(
                KSX::Fragment(vec![f::kxr(
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
            f::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![],
                    vec![f::kxr(KSX::Text(String::from("bar")), ((1, 6), (1, 8)))],
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
            f::kxr(
                KSX::OpenElement(
                    String::from("foo"),
                    vec![(
                        String::from("bar"),
                        Some(f::xr(
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
            f::kxr(
                KSX::ClosedElement(
                    String::from("foo"),
                    vec![(
                        String::from("bar"),
                        Some(f::xr(
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
            f::kxr(
                KSX::ClosedElement(String::from("foo"), vec![(String::from("bar"), None)],),
                ((1, 1), (1, 11))
            )
        );
    }
}
