use crate::{matcher as m, position::Decrement};
use combine::{attempt, between, choice, parser, sep_end_by, value, Parser, Stream};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum TypeExpression {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,
    Identifier(String),
    Group(Box<TypeExpression>),
    DotAccess(Box<TypeExpression>, String),
    Function(Vec<TypeExpression>, Box<TypeExpression>),
    // View(Vec<(String, TypeExpression)>),
}

fn primitive<T>() -> impl Parser<T, Output = TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    choice((
        m::keyword("nil").with(value(TypeExpression::Nil)),
        m::keyword("boolean").with(value(TypeExpression::Boolean)),
        m::keyword("integer").with(value(TypeExpression::Integer)),
        m::keyword("float").with(value(TypeExpression::Float)),
        m::keyword("string").with(value(TypeExpression::String)),
        m::keyword("style").with(value(TypeExpression::Style)),
        m::keyword("element").with(value(TypeExpression::Element)),
    ))
}

fn identifier<T>() -> impl Parser<T, Output = TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    m::standard_identifier().map(|(x, _)| TypeExpression::Identifier(x))
}

fn group<T, P>(parser: P) -> impl Parser<T, Output = TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = TypeExpression>,
{
    between(m::symbol('('), m::symbol(')'), parser)
        .map(|inner| TypeExpression::Group(Box::new(inner)))
}

fn dot_access<T, P>(parser: P) -> impl Parser<T, Output = TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = TypeExpression>,
{
    m::folding(
        parser,
        m::symbol('.').with(m::standard_identifier()),
        |lhs, (rhs, _)| TypeExpression::DotAccess(Box::new(lhs), rhs),
    )
}

fn function<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = TypeExpression>,
{
    (
        attempt(
            between(
                m::symbol('('),
                m::symbol(')'),
                sep_end_by(parser(), m::symbol(',')),
            )
            .skip(m::glyph("->")),
        ),
        parser(),
    )
        .map(|(parameters, result)| TypeExpression::Function(parameters, Box::new(result)))
}

// TODO: use this for lists ([], [][][])
fn type_expression_0<T>() -> impl Parser<T, Output = TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type_expression_1()
}

fn type_expression_1<T>() -> impl Parser<T, Output = TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    dot_access(type_expression_2())
}

fn type_expression_2<T>() -> impl Parser<T, Output = TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    choice((
        function(type_expression),
        group(type_expression()),
        primitive(),
        identifier(),
    ))
}

parser! {
    pub fn type_expression[T]()(T) -> TypeExpression
    where
        [T: Stream<Token = char>, T::Position: Copy + Debug + Decrement]
    {
        type_expression_0()
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        types::type_expression::{self, TypeExpression},
        ParseResult,
    };
    use combine::{stream::position::Stream, EasyParser, Parser};

    fn parse(s: &str) -> ParseResult<TypeExpression> {
        type_expression::type_expression().easy_parse(Stream::new(s))
    }

    #[test]
    fn nil() {
        assert_eq!(parse("nil").unwrap().0, TypeExpression::Nil);
    }

    #[test]
    fn boolean() {
        assert_eq!(parse("boolean").unwrap().0, TypeExpression::Boolean);
    }

    #[test]
    fn integer() {
        assert_eq!(parse("integer").unwrap().0, TypeExpression::Integer);
    }

    #[test]
    fn float() {
        assert_eq!(parse("float").unwrap().0, TypeExpression::Float);
    }

    #[test]
    fn string() {
        assert_eq!(parse("string").unwrap().0, TypeExpression::String);
    }

    #[test]
    fn style() {
        assert_eq!(parse("style").unwrap().0, TypeExpression::Style);
    }

    #[test]
    fn element() {
        assert_eq!(parse("element").unwrap().0, TypeExpression::Element);
    }

    #[test]
    fn identifier() {
        assert_eq!(
            parse("foo").unwrap().0,
            TypeExpression::Identifier(String::from("foo"))
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            parse("(nil)").unwrap().0,
            TypeExpression::Group(Box::new(TypeExpression::Nil))
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            parse("(nil, boolean) -> nil").unwrap().0,
            TypeExpression::Function(
                vec![TypeExpression::Nil, TypeExpression::Boolean],
                Box::new(TypeExpression::Nil)
            )
        );
    }

    #[test]
    fn function_empty_parameters() {
        assert_eq!(
            parse("() -> nil").unwrap().0,
            TypeExpression::Function(vec![], Box::new(TypeExpression::Nil))
        );
    }

    #[test]
    fn dot_access() {
        let parse = |s| type_expression::type_expression().parse(s);

        assert_eq!(
            parse("nil.foo").unwrap().0,
            TypeExpression::DotAccess(Box::new(TypeExpression::Nil), String::from("foo"))
        );
    }
}
