use crate::{
    ast::{BinaryOperator, Expression, ExpressionNode},
    common::position::Decrement,
    parser::matcher as m,
};
use combine::{chainl1, chainr1, choice, Parser, Stream};
use std::fmt::Debug;

fn binary_operation<T, U>(
    o: BinaryOperator,
) -> impl FnMut(U) -> Box<dyn Fn(ExpressionNode<T, ()>, ExpressionNode<T, ()>) -> ExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    move |_| {
        Box::new(move |lhs, rhs| {
            let range = lhs.node().range() + rhs.node().range();

            ExpressionNode::raw(
                Expression::BinaryOperation(o, Box::new(lhs), Box::new(rhs)),
                range,
            )
        })
    }
}

pub fn logical<T, P>(parser: P) -> impl Parser<T, Output = ExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = ExpressionNode<T, ()>>,
{
    let and = || {
        chainl1(
            parser,
            m::glyph("&&").map(binary_operation(BinaryOperator::And)),
        )
    };
    let or = || {
        chainl1(
            and(),
            m::glyph("||").map(binary_operation(BinaryOperator::Or)),
        )
    };

    or()
}

pub fn comparative<T, P>(parser: P) -> impl Parser<T, Output = ExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = ExpressionNode<T, ()>>,
{
    chainl1(
        parser,
        choice((
            m::glyph("==").map(binary_operation(BinaryOperator::Equal)),
            m::glyph("!=").map(binary_operation(BinaryOperator::NotEqual)),
        )),
    )
}

pub fn relational<T, P>(parser: P) -> impl Parser<T, Output = ExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = ExpressionNode<T, ()>>,
{
    chainl1(
        parser,
        choice((
            m::glyph("<=").map(binary_operation(BinaryOperator::LessThanOrEqual)),
            m::symbol('<').map(binary_operation(BinaryOperator::LessThan)),
            m::glyph(">=").map(binary_operation(BinaryOperator::GreaterThanOrEqual)),
            m::symbol('>').map(binary_operation(BinaryOperator::GreaterThan)),
        )),
    )
}

pub fn arithmetic<T, P>(parser: P) -> impl Parser<T, Output = ExpressionNode<T, ()>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
    P: Parser<T, Output = ExpressionNode<T, ()>>,
{
    let exponent = || {
        chainr1(
            parser,
            m::symbol('^').map(binary_operation(BinaryOperator::Exponent)),
        )
    };
    let multiply_or_divide = || {
        chainl1(
            exponent(),
            choice((
                m::symbol('*').map(binary_operation(BinaryOperator::Multiply)),
                m::symbol('/').map(binary_operation(BinaryOperator::Divide)),
            )),
        )
    };
    let add_or_subtract = || {
        chainl1(
            multiply_or_divide(),
            choice((
                m::symbol('+').map(binary_operation(BinaryOperator::Add)),
                m::symbol('-').map(binary_operation(BinaryOperator::Subtract)),
            )),
        )
    };

    add_or_subtract()
}
