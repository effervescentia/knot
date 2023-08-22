use crate::expression::{self, Expression};
use crate::matcher as m;
use combine::{attempt, between, choice, many, many1, none_of, optional, parser, Parser, Stream};

#[derive(Debug, PartialEq)]
pub enum KSX {
    Fragment(Vec<KSX>),
    Element(String, Vec<(String, Option<Expression>)>, Vec<KSX>),
    Inline(Expression),
    Text(String),
}

fn fragment<T>() -> impl Parser<T, Output = KSX>
where
    T: Stream<Token = char>,
{
    between(
        m::glyph("<>"),
        m::glyph("</>"),
        many::<Vec<_>, _, _>(child()),
    )
    .map(KSX::Fragment)
}

pub fn element<T>() -> impl Parser<T, Output = KSX>
where
    T: Stream<Token = char>,
{
    let attribute = (
        m::standard_identifier(),
        optional(m::symbol('=').with(expression::ksx_term())),
    );

    (
        attempt(m::symbol('<').with(m::standard_identifier())),
        many::<Vec<_>, _, _>(attribute),
        choice((
            m::glyph("/>").map(|_| Vec::<KSX>::new()),
            between(
                m::symbol('>'),
                (m::glyph("</"), m::standard_identifier(), m::symbol('>')),
                many(child()),
            ),
        )),
    )
        .map(|(name, attributes, children)| KSX::Element(name, attributes, children))
}

fn inline<T>() -> impl Parser<T, Output = KSX>
where
    T: Stream<Token = char>,
{
    between(m::symbol('{'), m::symbol('}'), expression::expression()).map(KSX::Inline)
}

fn text<T>() -> impl Parser<T, Output = KSX>
where
    T: Stream<Token = char>,
{
    m::lexeme(many1(none_of(vec!['<', '{']))).map(KSX::Text)
}

fn child_<T>() -> impl Parser<T, Output = KSX>
where
    T: Stream<Token = char>,
{
    choice((ksx(), inline(), text()))
}

parser! {
    fn child[T]()(T) -> KSX
    where
        [T: Stream<Token = char>]
    {
        child_()
    }
}

fn ksx_<T>() -> impl Parser<T, Output = KSX>
where
    T: Stream<Token = char>,
{
    choice((fragment(), element()))
}

parser! {
    pub fn ksx[T]()(T) -> KSX
    where
        [T: Stream<Token = char>]
    {
        ksx_()
    }
}

#[cfg(test)]
mod tests {
    use crate::expression::Expression;
    use crate::ksx::{ksx, KSX};
    use crate::primitive::Primitive;
    use combine::Parser;

    #[test]
    fn ksx_fragment() {
        let parse = |s| ksx().parse(s);

        assert_eq!(parse("<></>").unwrap().0, KSX::Fragment(vec![]));
    }

    #[test]
    fn ksx_element() {
        let parse = |s| ksx().parse(s);

        assert_eq!(
            parse("<foo />").unwrap().0,
            KSX::Element(String::from("foo"), vec![], vec![])
        );
        assert_eq!(
            parse("<foo></foo>").unwrap().0,
            KSX::Element(String::from("foo"), vec![], vec![])
        );
    }

    #[test]
    fn ksx_nested() {
        let parse = |s| ksx().parse(s);

        assert_eq!(
            parse("<><></></>").unwrap().0,
            KSX::Fragment(vec![KSX::Fragment(vec![])])
        );
        assert_eq!(
            parse("<><foo /></>").unwrap().0,
            KSX::Fragment(vec![KSX::Element(String::from("foo"), vec![], vec![])])
        );
        assert_eq!(
            parse("<foo><></></foo>").unwrap().0,
            KSX::Element(String::from("foo"), vec![], vec![KSX::Fragment(vec![])])
        );
        assert_eq!(
            parse("<foo><bar /></foo>").unwrap().0,
            KSX::Element(
                String::from("foo"),
                vec![],
                vec![KSX::Element(String::from("bar"), vec![], vec![])]
            )
        );
    }

    #[test]
    fn ksx_inline() {
        let parse = |s| ksx().parse(s);

        assert_eq!(
            parse("<>{nil}</>").unwrap().0,
            KSX::Fragment(vec![KSX::Inline(Expression::Primitive(Primitive::Nil))])
        );
        assert_eq!(
            parse("<foo>{nil}</foo>").unwrap().0,
            KSX::Element(
                String::from("foo"),
                vec![],
                vec![KSX::Inline(Expression::Primitive(Primitive::Nil))]
            )
        );
    }

    #[test]
    fn ksx_text() {
        let parse = |s| ksx().parse(s);

        assert_eq!(
            parse("<>foo</>").unwrap().0,
            KSX::Fragment(vec![KSX::Text(String::from("foo"))])
        );
        assert_eq!(
            parse("<foo>bar</foo>").unwrap().0,
            KSX::Element(
                String::from("foo"),
                vec![],
                vec![KSX::Text(String::from("bar"))]
            )
        );
    }

    #[test]
    fn ksx_attribute() {
        let parse = |s| ksx().parse(s);

        assert_eq!(
            parse("<foo bar />").unwrap().0,
            KSX::Element(
                String::from("foo"),
                vec![(String::from("bar"), None)],
                vec![]
            )
        );
        assert_eq!(
            parse("<foo bar=nil />").unwrap().0,
            KSX::Element(
                String::from("foo"),
                vec![(
                    String::from("bar"),
                    Some(Expression::Primitive(Primitive::Nil))
                )],
                vec![]
            )
        );
        assert_eq!(
            parse("<foo bar=nil></foo>").unwrap().0,
            KSX::Element(
                String::from("foo"),
                vec![(
                    String::from("bar"),
                    Some(Expression::Primitive(Primitive::Nil))
                )],
                vec![]
            )
        );
    }
}
