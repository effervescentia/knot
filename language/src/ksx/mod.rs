extern crate combine;
use crate::matcher as m;
mod child;
use combine::parser::combinator::lazy;
use combine::{between, choice, many, optional, parser, Parser, Stream};

#[derive(Debug, PartialEq)]
pub enum KSX<T> {
    Fragment(Vec<child::Child<T>>),
    Element(String, Vec<(String, Option<T>)>, Vec<child::Child<T>>),
}

pub fn fragment<T, R, P>(expression: impl Fn() -> P) -> impl Parser<T, Output = KSX<R>>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = R>,
{
    between(
        m::glyph("<>"),
        m::glyph("</>"),
        many::<Vec<_>, _, _>(child::child(expression)),
    )
    .map(KSX::Fragment)
}

pub fn element<T, R, P>(expression: impl Fn() -> P) -> impl Parser<T, Output = KSX<R>>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = R>,
{
    (
        m::symbol('<').with(m::standard_identifier()),
        many::<Vec<_>, _, _>((
            m::standard_identifier(),
            optional(m::symbol('=').with(expression())),
        )),
        choice((
            m::glyph("/>").map(|_| Vec::<child::Child<R>>::new()),
            between(
                m::symbol('>'),
                (m::glyph("</"), m::standard_identifier(), m::symbol('>')),
                many(child::child(expression)),
            ),
        )),
    )
        .map(|(name, attributes, children)| KSX::Element(name, attributes, children))
}

parser! {
    pub fn ksx[T, R, P, F](expression: F)(T) -> KSX<R>
    where
        [T: Stream<Token = char>, P: Parser<T, Output = R>, F: Fn() -> P]
    {
        // let child = ||choice((child::inline(parser()), child::text(), combine::parser(|input|child::node(||parser()).parse_stream(input).into_result())));
        // let child = ||choice((child::inline(parser()), child::text(), child::node(|| parser())));

        let fragment = || fragment(||expression());
        let element = || element(||expression());
        // let fragment = ||between(
        //     m::glyph("<>"),
        //     m::glyph("</>"),
        //     many::<Vec<_>, _, _>(child()),
        // )
        // .map(KSX::Fragment);
    // let element = (
    //     m::symbol('<').with(m::standard_identifier()),
    //     many::<Vec<_>, _, _>((
    //         m::standard_identifier(),
    //         optional(m::symbol('=').with(parser())),
    //     )),
    //     choice((
    //         m::glyph("/>").map(|_| Vec::<child::Child<R>>::new()),
    //         between(
    //             m::symbol('>'),
    //             (m::glyph("</"), m::standard_identifier(), m::symbol('>')),
    //             many(child()),
    //         ),
    //     )),
    // )
    //     .map(|(name, attributes, children)| KSX::Element(name, attributes, children));

        lazy(move ||choice((fragment(), element())))
    }
}

#[cfg(test)]
mod tests {
    use crate::ksx::{self, KSX};
    use crate::matcher;
    use combine::{Parser, Stream};

    const MOCK_TOKEN: &str = "__mock__";

    #[derive(Debug, PartialEq)]
    struct MockResult;

    fn mock<T>() -> impl Parser<T, Output = MockResult>
    where
        T: Stream<Token = char>,
    {
        matcher::keyword(MOCK_TOKEN).map(|_| MockResult)
    }

    #[test]
    fn ksx_child_text() {
        use ksx::child::{self, Child};

        let parse = |s| child::text::<_, MockResult>().parse(s);

        assert_eq!(parse("abc").unwrap().0, Child::Text(String::from("abc")));
    }

    #[test]
    fn ksx_child_inline() {
        use ksx::child::{self, Child};

        let parse = |s| child::inline(mock()).parse(s);

        let input = format!("{{{}}}", MOCK_TOKEN);
        assert_eq!(parse(input.as_str()).unwrap().0, Child::Inline(MockResult));
    }

    // #[test]
    // fn ksx_child() {
    //     use ksx::child::{self, Child};

    //     let parse = |s| child::child(|| Box::new(mock)).parse(s);

    //     let input = format!("{{{}}}", MOCK_TOKEN);
    //     assert_eq!(parse(input.as_str()).unwrap().0, Child::Inline(MockResult));
    //     assert_eq!(parse("abc").unwrap().0, Child::Text(String::from("abc")));
    // }

    // #[test]
    // fn ksx_fragment() {
    //     let parse = |s| ksx::fragment(|| Box::new(mock)).parse(s);

    //     assert_eq!(parse("<></>").unwrap().0, KSX::Fragment(Vec::new()));
    //     let input = format!("<>{{{}}}</>", MOCK_TOKEN);
    //     assert_eq!(
    //         parse(input.as_str()).unwrap().0,
    //         KSX::Fragment(vec![ksx::child::Child::Inline(MockResult)])
    //     );
    // }

    // #[test]
    // fn ksx_element() {
    //     let parse = |s| ksx::element(|| Box::new(mock)).parse(s);

    //     assert_eq!(
    //         parse("<foo />").unwrap().0,
    //         KSX::Element(String::from("foo"), Vec::new(), Vec::new())
    //     );
    //     assert_eq!(
    //         parse("<foo></foo>").unwrap().0,
    //         KSX::Element(String::from("foo"), Vec::new(), Vec::new())
    //     );
    //     let input = format!("<foo>{{{}}}</foo>", MOCK_TOKEN);
    //     assert_eq!(
    //         parse(input.as_str()).unwrap().0,
    //         KSX::Element(
    //             String::from("foo"),
    //             Vec::new(),
    //             vec![ksx::child::Child::Inline(MockResult)]
    //         )
    //     );
    // }

    #[test]
    fn ksx_attribute() {
        let parse = |s| ksx::ksx(mock).parse(s);

        assert_eq!(
            parse("<foo bar />").unwrap().0,
            KSX::Element(
                String::from("foo"),
                vec![(String::from("bar"), None)],
                Vec::new()
            )
        );
        let input = format!("<foo bar={} />", MOCK_TOKEN);
        assert_eq!(
            parse(input.as_str()).unwrap().0,
            KSX::Element(
                String::from("foo"),
                vec![(String::from("bar"), Some(MockResult))],
                Vec::new()
            )
        );
        let input = format!("<foo bar={}></foo>", MOCK_TOKEN);
        assert_eq!(
            parse(input.as_str()).unwrap().0,
            KSX::Element(
                String::from("foo"),
                vec![(String::from("bar"), Some(MockResult))],
                Vec::new()
            )
        );
    }

    #[test]
    fn ksx() {
        let parse = |s| ksx::ksx(mock).parse(s);

        assert_eq!(parse("<></>").unwrap().0, KSX::Fragment(Vec::new()));
        assert_eq!(
            parse("<foo />").unwrap().0,
            KSX::Element(String::from("foo"), Vec::new(), Vec::new())
        );
    }
}
