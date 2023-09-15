use crate::{
    parser::matcher as m,
    parser::{node::Node, position::Decrement, range::Range},
};
use combine::{attempt, choice, parser, sep_end_by, Parser, Stream};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum TypeExpression<T> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,
    Identifier(String),
    Group(Box<T>),
    DotAccess(Box<T>, String),
    Function(Vec<T>, Box<T>),
    // View(Vec<(String, TypeExpression)>),
}

#[derive(Debug, PartialEq)]
pub struct TypeExpressionNode<T, C>(pub Node<TypeExpression<TypeExpressionNode<T, C>>, T, C>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T, C> TypeExpressionNode<T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn node(&self) -> &Node<TypeExpression<TypeExpressionNode<T, C>>, T, C> {
        &self.0
    }
}

impl<T> TypeExpressionNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: TypeExpression<TypeExpressionNode<T, ()>>, range: Range<T>) -> Self {
        Self(Node::raw(x, range))
    }
}

fn primitive<T>() -> impl Parser<T, Output = TypeExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn bind<U>(
        s: &'static str,
        f: impl Fn() -> TypeExpression<TypeExpressionNode<U, ()>>,
    ) -> impl Parser<U, Output = TypeExpressionNode<U, ()>>
    where
        U: Stream<Token = char>,
        U::Position: Copy + Debug + Decrement,
    {
        m::keyword(s).map(move |(_, range)| TypeExpressionNode::raw(f(), range))
    }

    choice((
        bind("nil", || TypeExpression::Nil),
        bind("boolean", || TypeExpression::Boolean),
        bind("integer", || TypeExpression::Integer),
        bind("float", || TypeExpression::Float),
        bind("string", || TypeExpression::String),
        bind("style", || TypeExpression::Style),
        bind("element", || TypeExpression::Element),
    ))
}

fn identifier<T>() -> impl Parser<T, Output = TypeExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    m::standard_identifier()
        .map(|(x, range)| TypeExpressionNode::raw(TypeExpression::Identifier(x), range))
}

fn group<T, P>(parser: P) -> impl Parser<T, Output = TypeExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = TypeExpressionNode<T, ()>>,
{
    m::between(m::symbol('('), m::symbol(')'), parser).map(|(inner, range)| {
        TypeExpressionNode::raw(TypeExpression::Group(Box::new(inner)), range)
    })
}

fn dot_access<T, P>(parser: P) -> impl Parser<T, Output = TypeExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = TypeExpressionNode<T, ()>>,
{
    m::folding(
        parser,
        m::symbol('.').with(m::standard_identifier()),
        |lhs, (rhs, end)| {
            let range = lhs.0.range() + &end;
            TypeExpressionNode::raw(TypeExpression::DotAccess(Box::new(lhs), rhs), range)
        },
    )
}

fn function<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = TypeExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = TypeExpressionNode<T, ()>>,
{
    (
        attempt(
            m::between(
                m::symbol('('),
                m::symbol(')'),
                sep_end_by(parser(), m::symbol(',')),
            )
            .skip(m::glyph("->")),
        ),
        parser(),
    )
        .map(|((parameters, start), result)| {
            let range = &start + result.0.range();
            TypeExpressionNode::raw(
                TypeExpression::Function(parameters, Box::new(result)),
                range,
            )
        })
}

fn type_expression_2<T>() -> impl Parser<T, Output = TypeExpressionNode<T, ()>>
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

fn type_expression_1<T>() -> impl Parser<T, Output = TypeExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    dot_access(type_expression_2())
}

// TODO: use this for lists ([], [][][])
fn type_expression_0<T>() -> impl Parser<T, Output = TypeExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type_expression_1()
}

parser! {
    pub fn type_expression[T]()(T) -> TypeExpressionNode<T, ()>
    where
        [T: Stream<Token = char>, T::Position: Copy + Debug + Decrement]
    {
        type_expression_0()
    }
}

#[cfg(test)]
mod tests {
    use super::{TypeExpression, TypeExpressionNode};
    use crate::{
        parser::{CharStream, ParseResult},
        test::fixture as f,
    };
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> ParseResult<TypeExpressionNode<CharStream, ()>> {
        super::type_expression().easy_parse(Stream::new(s))
    }

    #[test]
    fn nil() {
        assert_eq!(
            parse("nil").unwrap().0,
            f::n::txr(TypeExpression::Nil, ((1, 1), (1, 3)))
        );
    }

    #[test]
    fn boolean() {
        assert_eq!(
            parse("boolean").unwrap().0,
            f::n::txr(TypeExpression::Boolean, ((1, 1), (1, 7)))
        );
    }

    #[test]
    fn integer() {
        assert_eq!(
            parse("integer").unwrap().0,
            f::n::txr(TypeExpression::Integer, ((1, 1), (1, 7)))
        );
    }

    #[test]
    fn float() {
        assert_eq!(
            parse("float").unwrap().0,
            f::n::txr(TypeExpression::Float, ((1, 1), (1, 5)))
        );
    }

    #[test]
    fn string() {
        assert_eq!(
            parse("string").unwrap().0,
            f::n::txr(TypeExpression::String, ((1, 1), (1, 6)))
        );
    }

    #[test]
    fn style() {
        assert_eq!(
            parse("style").unwrap().0,
            f::n::txr(TypeExpression::Style, ((1, 1), (1, 5)))
        );
    }

    #[test]
    fn element() {
        assert_eq!(
            parse("element").unwrap().0,
            f::n::txr(TypeExpression::Element, ((1, 1), (1, 7)))
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            parse("foo").unwrap().0,
            f::n::txr(
                TypeExpression::Identifier(String::from("foo")),
                ((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            parse("(nil)").unwrap().0,
            f::n::txr(
                TypeExpression::Group(Box::new(f::n::txr(TypeExpression::Nil, ((1, 2), (1, 4))))),
                ((1, 1), (1, 5))
            )
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            parse("(nil, boolean) -> nil").unwrap().0,
            f::n::txr(
                TypeExpression::Function(
                    vec![
                        f::n::txr(TypeExpression::Nil, ((1, 2), (1, 4))),
                        f::n::txr(TypeExpression::Boolean, ((1, 7), (1, 13)))
                    ],
                    Box::new(f::n::txr(TypeExpression::Nil, ((1, 19), (1, 21))))
                ),
                ((1, 1), (1, 21))
            )
        );
    }

    #[test]
    fn function_empty_parameters() {
        assert_eq!(
            parse("() -> nil").unwrap().0,
            f::n::txr(
                TypeExpression::Function(
                    vec![],
                    Box::new(f::n::txr(TypeExpression::Nil, ((1, 7), (1, 9))))
                ),
                ((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            parse("nil.foo").unwrap().0,
            f::n::txr(
                TypeExpression::DotAccess(
                    Box::new(f::n::txr(TypeExpression::Nil, ((1, 1), (1, 3)))),
                    String::from("foo")
                ),
                ((1, 1), (1, 7))
            )
        );
    }
}
