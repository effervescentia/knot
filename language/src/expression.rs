use super::statement::{self, Statement};
use crate::ksx::{self, KSX};
use crate::matcher as m;
use crate::primitive::{self, Primitive};
use combine::{
    attempt, between, chainl1, chainr1, choice, many, parser, sep_end_by, Parser, Stream,
};

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum BinaryOperator {
    And,
    Or,

    Equal,
    NotEqual,

    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,

    Add,
    Subtract,
    Multiply,
    Divide,
    Exponent,
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum UnaryOperator {
    Not,
    Absolute,
    Negate,
}

#[derive(Debug, PartialEq)]
pub enum Expression {
    Primitive(Primitive),
    Identifier(String),
    Group(Box<Expression>),
    Closure(Vec<Statement<Expression>>),
    UnaryOperation(UnaryOperator, Box<Expression>),
    BinaryOperation(BinaryOperator, Box<Expression>, Box<Expression>),
    DotAccess(Box<Expression>, String),
    FunctionCall(Box<Expression>, Vec<Expression>),
    Style(Vec<(String, Expression)>),
    KSX(Box<KSX>),
}

fn primitive<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    m::lexeme(primitive::primitive()).map(Expression::Primitive)
}

fn group<T, P>(parser: P) -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = Expression>,
{
    between(m::symbol('('), m::symbol(')'), parser).map(|inner| Expression::Group(Box::new(inner)))
}

fn identifier<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    m::lexeme(m::standard_identifier()).map(Expression::Identifier)
}

fn closure<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = Expression>,
{
    between(
        m::symbol('{'),
        m::symbol('}'),
        many::<Vec<_>, _, _>(statement::statement(parser)),
    )
    .map(Expression::Closure)
}

fn unary_operation<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = Expression>,
{
    let operation = |c, o| m::symbol(c).with(parser()).map(move |r| (o, r));

    choice((
        operation('!', UnaryOperator::Not),
        operation('+', UnaryOperator::Absolute),
        operation('-', UnaryOperator::Negate),
    ))
    .map(|(op, r)| Expression::UnaryOperation(op, Box::new(r)))
    .or(parser())
}

fn binary_operation(
    o: BinaryOperator,
) -> impl FnMut(()) -> Box<dyn Fn(Expression, Expression) -> Expression> {
    move |_| Box::new(move |lhs, rhs| Expression::BinaryOperation(o, Box::new(lhs), Box::new(rhs)))
}

fn logical<T, P>(parser: P) -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = Expression>,
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

fn comparative<T, P>(parser: P) -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = Expression>,
{
    chainl1(
        parser,
        choice((
            m::glyph("==").map(binary_operation(BinaryOperator::Equal)),
            m::glyph("!=").map(binary_operation(BinaryOperator::NotEqual)),
        )),
    )
}

fn relational<T, P>(parser: P) -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = Expression>,
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

fn arithmetic<T, P>(parser: P) -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = Expression>,
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

fn dot_access<T, P>(parser: P) -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = Expression>,
{
    m::folding(
        parser,
        m::symbol('.').with(m::standard_identifier()),
        |lhs, rhs| Expression::DotAccess(Box::new(lhs), rhs),
    )
}

fn function_call<T, P1, P2>(lhs: P1, rhs: P2) -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
    P1: Parser<T, Output = Expression>,
    P2: Parser<T, Output = Expression>,
{
    m::folding(
        lhs,
        between(
            m::symbol('('),
            m::symbol(')'),
            sep_end_by::<Vec<_>, _, _, _>(rhs, m::symbol(',')),
        ),
        |acc, args| Expression::FunctionCall(Box::new(acc), args),
    )
}

fn style_literal<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = Expression>,
{
    let style_rule =
        || (m::standard_identifier(), m::symbol(':'), parser()).map(|(lhs, _, rhs)| (lhs, rhs));

    between(
        m::symbol('{'),
        m::symbol('}'),
        sep_end_by(style_rule(), m::symbol(',')),
    )
    .map(Expression::Style)
}

fn style<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
    P: Parser<T, Output = Expression>,
{
    attempt(m::keyword("style").with(style_literal(parser)))
}

fn ksx<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    ksx::ksx().map(|ksx| Expression::KSX(Box::new(ksx)))
}

fn expression_8<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    choice((attempt(primitive()), style(expression), ksx(), identifier()))
}

fn expression_7<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    choice((closure(expression), group(expression()), expression_8()))
}

fn expression_6<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    dot_access(expression_7())
}

fn expression_5<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    function_call(expression_6(), expression())
}

fn expression_4<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    unary_operation(expression_5)
}

fn expression_3<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    arithmetic(expression_4())
}

fn expression_2<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    relational(expression_3())
}

fn expression_1<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    comparative(expression_2())
}

fn expression_0<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    logical(expression_1())
}

parser! {
    pub fn expression[T]()(T) -> Expression
    where
        [T: Stream<Token = char>]
    {
        expression_0()
    }
}

pub fn ksx_term<T>() -> impl Parser<T, Output = Expression>
where
    T: Stream<Token = char>,
{
    unary_operation(|| function_call(expression_8(), expression()))
}

#[cfg(test)]
mod tests {
    use crate::expression::{expression, BinaryOperator, Expression, UnaryOperator};
    use crate::primitive::Primitive;
    use crate::statement::Statement;
    use combine::Parser;

    #[test]
    fn expression_primitive() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("nil").unwrap().0,
            Expression::Primitive(Primitive::Nil)
        );
        assert_eq!(
            parse("true").unwrap().0,
            Expression::Primitive(Primitive::Boolean(true))
        );
        assert_eq!(
            parse("false").unwrap().0,
            Expression::Primitive(Primitive::Boolean(false))
        );
        assert_eq!(
            parse("123").unwrap().0,
            Expression::Primitive(Primitive::Integer(123))
        );
        assert_eq!(
            parse("123.456").unwrap().0,
            Expression::Primitive(Primitive::Float(123.456, 6))
        );
        assert_eq!(
            parse("\"foo\"").unwrap().0,
            Expression::Primitive(Primitive::String(String::from("foo")))
        );
    }

    #[test]
    fn expression_identifier() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("foo").unwrap().0,
            Expression::Identifier(String::from("foo"))
        );
    }

    #[test]
    fn expression_group() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("(nil)").unwrap().0,
            Expression::Group(Box::new(Expression::Primitive(Primitive::Nil)))
        );
    }

    #[test]
    fn expression_closure() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("{ nil; nil }").unwrap().0,
            Expression::Closure(vec![
                Statement::Effect(Expression::Primitive(Primitive::Nil)),
                Statement::Effect(Expression::Primitive(Primitive::Nil))
            ])
        );
        assert_eq!(parse("{}").unwrap().0, Expression::Closure(vec![]));
    }

    #[test]
    fn expression_unary_operation() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("!nil").unwrap().0,
            Expression::UnaryOperation(
                UnaryOperator::Not,
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("+nil").unwrap().0,
            Expression::UnaryOperation(
                UnaryOperator::Absolute,
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("-nil").unwrap().0,
            Expression::UnaryOperation(
                UnaryOperator::Negate,
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
    }

    #[test]
    fn expression_binary_operation() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("nil + nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::Add,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("nil - nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::Subtract,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("nil * nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::Multiply,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("nil / nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::Divide,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("nil ^ nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::Exponent,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("nil < nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::LessThan,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("nil <= nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::LessThanOrEqual,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("nil > nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::GreaterThan,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("nil >= nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::GreaterThanOrEqual,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("nil == nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::Equal,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("nil != nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::NotEqual,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("nil && nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::And,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
        assert_eq!(
            parse("nil || nil").unwrap().0,
            Expression::BinaryOperation(
                BinaryOperator::Or,
                Box::new(Expression::Primitive(Primitive::Nil)),
                Box::new(Expression::Primitive(Primitive::Nil)),
            )
        );
    }

    #[test]
    fn expression_dot_access() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("nil.foo").unwrap().0,
            Expression::DotAccess(
                Box::new(Expression::Primitive(Primitive::Nil)),
                String::from("foo")
            )
        );
    }

    #[test]
    fn expression_function_call() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("nil(nil, nil)").unwrap().0,
            Expression::FunctionCall(
                Box::new(Expression::Primitive(Primitive::Nil)),
                vec![
                    Expression::Primitive(Primitive::Nil),
                    Expression::Primitive(Primitive::Nil)
                ]
            )
        );
        assert_eq!(
            parse("nil()").unwrap().0,
            Expression::FunctionCall(Box::new(Expression::Primitive(Primitive::Nil)), vec![])
        );
    }

    #[test]
    fn expression_style() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("style { foo: nil }").unwrap().0,
            Expression::Style(vec![(
                String::from("foo"),
                Expression::Primitive(Primitive::Nil)
            )])
        );
    }
}
