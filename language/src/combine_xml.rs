extern crate combine;
use combine::parser::char::{alpha_num, char, string};
use combine::{attempt, parser};
use combine::{between, choice, many, many1, none_of, parser::combinator::lazy, Parser, Stream};

#[derive(Debug, PartialEq)]
pub enum HTML {
    Text(String),
    Node(String, Vec<(String, String)>, Vec<HTML>),
}

pub fn text<T>() -> impl Parser<T, Output = HTML>
where
    T: Stream<Token = char>,
{
    many1(none_of(vec!['<', '{'])).map(HTML::Text)
}

pub fn node<T, P>(parser: P) -> impl Parser<T, Output = HTML>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = HTML>,
{
    // let attribute = {
    //     (
    //         many1::<String, _, _>(alpha_num()),
    //         char('='),
    //         between(char('"'), char('"'), many(none_of(vec!['"']))),
    //     )
    //         .map(|(key, _, value)| (key, value))
    // };

    // (
    //     between(
    //         char('<'),
    //         char('>'),
    //         (many1::<String, _, _>(alpha_num()), many(attribute)),
    //     ),
    //     many::<Vec<HTML>, _, _>(parser),
    //     between(string("</"), char('>'), many1::<String, _, _>(alpha_num())),
    // )
    //     .map(|((name, attributes), children, _)| HTML::Node(name, attributes, children))

    (
        attempt(between(
            char('<'),
            char('>'),
            many1::<String, _, _>(alpha_num()),
        )),
        many::<Vec<HTML>, _, _>(parser),
        between(string("</"), char('>'), many1::<String, _, _>(alpha_num())),
    )
        .map(|(name, children, _)| HTML::Node(name, Vec::new(), children))
}

fn html_<T>() -> impl Parser<T, Output = HTML>
where
    T: Stream<Token = char>,
{
    choice((node(html()), text()))
}

parser! {
    pub fn html[T]()(T) -> HTML
    where [T: Stream<Token=char>]
    {
        node(html_())
    }
}

// pub fn element<T, R, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = KSX<R>>
// where
//     T: Stream<Token = char>,
//     P: Parser<T, Output = R>,
// {
//     (
//         m::symbol('<').with(m::standard_identifier()),
//         many::<Vec<_>, _, _>((
//             m::standard_identifier(),
//             optional(m::symbol('=').with(parser())),
//         )),
//         choice((
//             m::glyph("/>").map(|_| Vec::<child::Child<R>>::new()),
//             between(
//                 m::symbol('>'),
//                 (m::glyph("</"), m::standard_identifier(), m::symbol('>')),
//                 many(child::child(parser)),
//             ),
//         )),
//     )
//         .map(|(name, attributes, children)| KSX::Element(name, attributes, children))
// }

// combine::parser! {
//     pub fn ksx[T, R, P, F](parser: F)(T) -> KSX<R>
//     where
//         [T: Stream<Token = char>, P: Parser<T, Output = R>, F: Fn() -> P]
//     {
//         // let child = ||choice((child::inline(parser()), child::text(), combine::parser(|input|child::node(||parser()).parse_stream(input).into_result())));
//         // let child = ||choice((child::inline(parser()), child::text(), child::node(|| parser())));

//         let fragment = || fragment(||parser());
//         let element = || element(||parser());
//         // let fragment = ||between(
//         //     m::glyph("<>"),
//         //     m::glyph("</>"),
//         //     many::<Vec<_>, _, _>(child()),
//         // )
//         // .map(KSX::Fragment);
//     // let element = (
//     //     m::symbol('<').with(m::standard_identifier()),
//     //     many::<Vec<_>, _, _>((
//     //         m::standard_identifier(),
//     //         optional(m::symbol('=').with(parser())),
//     //     )),
//     //     choice((
//     //         m::glyph("/>").map(|_| Vec::<child::Child<R>>::new()),
//     //         between(
//     //             m::symbol('>'),
//     //             (m::glyph("</"), m::standard_identifier(), m::symbol('>')),
//     //             many(child()),
//     //         ),
//     //     )),
//     // )
//     //     .map(|(name, attributes, children)| KSX::Element(name, attributes, children));

//         lazy(move ||choice((fragment(), element())))
//     }
// }

#[cfg(test)]
mod tests {
    use super::HTML;
    use crate::combine_knot::expression::{BinaryOperator, UnaryOperator};
    use crate::combine_knot::matcher;
    use combine::error::StringStreamError;
    use combine::{parser, Parser, Stream};

    // #[test]
    // fn ksx_child_text() {
    //     use ksx::child::{self, Child};

    //     let parse = |s| child::text::<_, MockResult>().parse(s);

    //     assert_eq!(parse("abc").unwrap().0, Child::Text(String::from("abc")));
    // }

    // #[test]
    // fn ksx_child_inline() {
    //     use ksx::child::{self, Child};

    //     let parse = |s| child::inline(mock()).parse(s);

    //     let input = format!("{{{}}}", MOCK_TOKEN);
    //     assert_eq!(parse(input.as_str()).unwrap().0, Child::Inline(MockResult));
    // }

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

    #[test]
    fn html() {
        let parse = |s| super::html().parse(s);

        assert_eq!(
            parse("<foo></foo>").unwrap().0,
            HTML::Node(String::from("foo"), Vec::new(), Vec::new())
        );
        // assert_eq!(
        //     parse("<foo></foo>").unwrap().0,
        //     HTML::Node(String::from("foo"), Vec::new(), Vec::new())
        // );
        // assert_eq!(
        //     parse("<foo></foo>").unwrap().0,
        //     KSX::Element(String::from("foo"), Vec::new(), Vec::new())
        // );
        // let input = format!("<foo>{{{}}}</foo>", MOCK_TOKEN);
        // assert_eq!(
        //     parse(input.as_str()).unwrap().0,
        //     KSX::Element(
        //         String::from("foo"),
        //         Vec::new(),
        //         vec![ksx::child::Child::Inline(MockResult)]
        //     )
        // );
    }

    // #[test]
    // fn ksx_attribute() {
    //     let parse = |s| ksx::ksx(mock).parse(s);

    //     assert_eq!(
    //         parse("<foo bar />").unwrap().0,
    //         KSX::Element(
    //             String::from("foo"),
    //             vec![(String::from("bar"), None)],
    //             Vec::new()
    //         )
    //     );
    //     let input = format!("<foo bar={} />", MOCK_TOKEN);
    //     assert_eq!(
    //         parse(input.as_str()).unwrap().0,
    //         KSX::Element(
    //             String::from("foo"),
    //             vec![(String::from("bar"), Some(MockResult))],
    //             Vec::new()
    //         )
    //     );
    //     let input = format!("<foo bar={}></foo>", MOCK_TOKEN);
    //     assert_eq!(
    //         parse(input.as_str()).unwrap().0,
    //         KSX::Element(
    //             String::from("foo"),
    //             vec![(String::from("bar"), Some(MockResult))],
    //             Vec::new()
    //         )
    //     );
    // }

    // #[test]
    // fn ksx() {
    //     let parse = |s| ksx::ksx(mock).parse(s);

    //     assert_eq!(parse("<></>").unwrap().0, KSX::Fragment(Vec::new()));
    //     assert_eq!(
    //         parse("<foo />").unwrap().0,
    //         KSX::Element(String::from("foo"), Vec::new(), Vec::new())
    //     );
    // }
}
