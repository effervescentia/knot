extern crate combine;
use crate::matcher as m;
use combine::{between, choice, many1, none_of, Parser, Stream};

#[derive(Debug, PartialEq)]
pub enum Child<T> {
    Node(super::KSX<T>),
    Text(String),
    Inline(T),
}

pub fn text<T, R>() -> impl Parser<T, Output = Child<R>>
where
    T: Stream<Token = char>,
{
    m::lexeme(many1(none_of(vec!['<', '{']))).map(Child::Text)
}

pub fn inline<T, R, P>(parser: P) -> impl Parser<T, Output = Child<R>>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = R>,
{
    between(m::symbol('{'), m::symbol('}'), parser).map(Child::Inline)
}

pub fn node<T, R, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = Child<R>>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = R>,
{
    super::ksx(parser).map(|res| Child::Node(res))
}

pub fn child<T, R, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = Child<R>>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = R>,
{
    // choice((inline(parser()), text(), node(parser)))
    // choice((inline(parser()), text(), node(parser)))
    choice((inline(parser()), text()))
}

#[cfg(test)]
mod tests {
    use crate::ksx::child;
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
    fn text() {
        use child::{self, Child};

        let parse = |s| child::text::<_, MockResult>().parse(s);

        assert_eq!(parse("abc").unwrap().0, Child::Text(String::from("abc")));
    }

    #[test]
    fn inline() {
        use child::{self, Child};

        let parse = |s| child::inline(mock()).parse(s);

        let input = format!("{{{}}}", MOCK_TOKEN);
        assert_eq!(parse(input.as_str()).unwrap().0, Child::Inline(MockResult));
    }

    // #[test]
    // fn child() {
    //     use ksx::child::{self, Child};

    //     let parse = |s| child::child(|| Box::new(mock)).parse(s);

    //     let input = format!("{{{}}}", MOCK_TOKEN);
    //     assert_eq!(parse(input.as_str()).unwrap().0, Child::Inline(MockResult));
    //     assert_eq!(parse("abc").unwrap().0, Child::Text(String::from("abc")));
    // }
}
