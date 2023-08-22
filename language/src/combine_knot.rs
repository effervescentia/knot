extern crate combine;

pub mod matcher {
    use combine::parser::char as p;
    use combine::{attempt, many, optional, satisfy, unexpected_any, value, Parser, Stream};

    pub fn lexeme<T, R, P>(parser: P) -> impl Parser<T, Output = R>
    where
        T: Stream<Token = char>,
        P: Parser<T, Output = R>,
    {
        parser.skip(p::spaces())
    }

    pub fn terminated<T, R, P>(parser: P) -> impl Parser<T, Output = R>
    where
        T: Stream<Token = char>,
        P: Parser<T, Output = R>,
    {
        parser.skip(optional(symbol(';')))
    }

    pub fn folding<T, R1, R2, P1, P2>(
        lhs: P1,
        rhs: P2,
        fold: impl Fn(R1, R2) -> R1,
    ) -> impl Parser<T, Output = R1>
    where
        T: Stream<Token = char>,
        P1: Parser<T, Output = R1>,
        P2: Parser<T, Output = R2>,
    {
        lhs.and(many::<Vec<_>, _, _>(rhs))
            .map(move |(lhs, args)| args.into_iter().fold(lhs, |acc, el| fold(acc, el)))
    }

    pub fn symbol<T>(c: char) -> impl Parser<T, Output = ()>
    where
        T: Stream<Token = char>,
    {
        lexeme(p::char(c)).with(value(()))
    }

    pub fn glyph<T>(glyph: &'static str) -> impl Parser<T, Output = ()>
    where
        T: Stream<Token = char>,
    {
        let mut chars = glyph.chars().into_iter();

        attempt(
            combine::many::<Vec<_>, _, _>(lexeme(satisfy(move |c| Some(c) == chars.next()))).then(
                |r| {
                    if r.len() == glyph.len() {
                        value(()).left()
                    } else {
                        unexpected_any("complete glyph not parsed").right()
                    }
                },
            ),
        )
    }

    pub fn keyword<T>(keyword: &'static str) -> impl Parser<T, Output = ()>
    where
        T: Stream<Token = char>,
    {
        lexeme(p::string(keyword)).with(value(()))
    }

    pub fn identifier<T>(prefix: impl Parser<T, Output = char>) -> impl Parser<T, Output = String>
    where
        T: Stream<Token = char>,
    {
        lexeme(prefix.then(|first| {
            many::<Vec<_>, _, _>(p::alpha_num().or(p::char('_'))).map(move |mut rest| {
                let mut result = vec![first];
                result.append(&mut rest);
                result.into_iter().collect::<String>()
            })
        }))
    }

    pub fn standard_identifier<T>() -> impl Parser<T, Output = String>
    where
        T: Stream<Token = char>,
    {
        identifier(p::alpha_num().or(p::char('_')))
    }
}

pub mod primitive {
    use combine::parser::char as p;
    use combine::{attempt, between, choice, many, many1, none_of, token, value, Parser, Stream};

    #[derive(Clone, Debug, PartialEq)]
    pub enum Primitive {
        Nil,
        Boolean(bool),
        Integer(i64),
        Float(f64, i32),
        String(String),
    }

    fn nil<T>() -> impl Parser<T, Output = Primitive>
    where
        T: Stream<Token = char>,
    {
        p::string("nil").with(value(Primitive::Nil))
    }

    fn boolean<T>() -> impl Parser<T, Output = Primitive>
    where
        T: Stream<Token = char>,
    {
        p::string("true")
            .with(value(true))
            .or(p::string("false").with(value(false)))
            .map(Primitive::Boolean)
    }

    fn integer<T>() -> impl Parser<T, Output = Primitive>
    where
        T: Stream<Token = char>,
    {
        many1::<String, _, _>(p::digit())
            .map(|chars| chars.parse::<i64>().unwrap())
            .map(Primitive::Integer)
    }

    fn float<T>() -> impl Parser<T, Output = Primitive>
    where
        T: Stream<Token = char>,
    {
        let integer = many1::<String, _, _>(p::digit());
        let fraction = many1::<String, _, _>(p::digit());

        (integer, token('.'), fraction).map(|(integer, _, fraction)| {
            if fraction.is_empty() {
                Primitive::Float(integer.parse::<f64>().unwrap(), integer.len() as i32)
            } else {
                let float = format!("{}.{}", integer, fraction);
                Primitive::Float(
                    float.parse::<f64>().unwrap(),
                    (integer.len() + fraction.len()) as i32,
                )
            }
        })
    }

    fn string<T>() -> impl Parser<T, Output = Primitive>
    where
        T: Stream<Token = char>,
    {
        between(token('"'), token('"'), many(none_of(vec!['"', '\\']))).map(Primitive::String)
    }

    pub fn primitive<T>() -> impl Parser<T, Output = Primitive>
    where
        T: Stream<Token = char>,
    {
        choice((nil(), boolean(), attempt(float()), integer(), string()))
    }
}

pub mod expression {
    use super::ksx::KSX;
    use super::matcher as m;
    use super::primitive::{self, Primitive};
    use super::statement::{self, Statement};
    use combine::{
        attempt, between, chainl1, chainr1, choice, many, parser, sep_by, Parser, Stream,
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
        KSX(KSX<Expression>),
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
        between(m::symbol('('), m::symbol(')'), parser)
            .map(|inner| Expression::Group(Box::new(inner)))
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
    }

    fn binary_operation(
        o: BinaryOperator,
    ) -> impl FnMut(()) -> Box<dyn Fn(Expression, Expression) -> Expression> {
        move |_| {
            Box::new(move |lhs, rhs| Expression::BinaryOperation(o, Box::new(lhs), Box::new(rhs)))
        }
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
                sep_by::<Vec<_>, _, _, _>(rhs, m::symbol(',')),
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
            sep_by(style_rule(), m::symbol(',')),
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

    fn expression_8<T>() -> impl Parser<T, Output = Expression>
    where
        T: Stream<Token = char>,
    {
        choice((attempt(primitive()), style(expression), identifier()))
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
        unary_operation(expression_5).or(expression_5())
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
}

pub mod ksx {
    use super::matcher as m;
    use combine::{
        between, choice, many, optional,
        parser::{self, combinator::lazy},
        Parser, Stream,
    };

    pub mod child {
        use super::super::matcher as m;
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
    }

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
}

pub mod statement {
    use super::matcher as m;
    use combine::{choice, Parser, Stream};

    #[derive(Debug, PartialEq)]
    pub enum Statement<T> {
        Effect(T),
        Variable(String, T),
    }

    fn effect<T, R, P>(parser: P) -> impl Parser<T, Output = Statement<R>>
    where
        T: Stream<Token = char>,
        P: Parser<T, Output = R>,
    {
        m::terminated(parser).map(|inner| Statement::Effect(inner))
    }

    fn variable<T, R, P>(parser: P) -> impl Parser<T, Output = Statement<R>>
    where
        T: Stream<Token = char>,
        P: Parser<T, Output = R>,
    {
        m::terminated((
            m::keyword("let"),
            m::standard_identifier(),
            m::symbol('='),
            parser,
        ))
        .map(|(_, name, _, value)| Statement::Variable(name, value))
    }

    pub fn statement<T, R, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = Statement<R>>
    where
        T: Stream<Token = char>,
        P: Parser<T, Output = R>,
    {
        choice((effect(parser()), variable(parser())))
    }
}

#[cfg(test)]
mod tests {
    use super::expression::{self, Expression};
    use super::ksx::{self, KSX};
    use super::primitive::{self, Primitive};
    use super::statement::{self, Statement};
    use crate::combine_knot::expression::{BinaryOperator, UnaryOperator};
    use crate::combine_knot::matcher;
    use combine::error::StringStreamError;
    use combine::{parser, Parser, Stream};

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
    fn lexeme() {
        let parse = |s| matcher::lexeme(mock()).parse(s);

        assert_eq!(parse(MOCK_TOKEN).unwrap().0, MockResult);

        let with_trailing_space = format!("{} ", MOCK_TOKEN);
        assert_eq!(parse(with_trailing_space.as_str()).unwrap().0, MockResult);
    }

    #[test]
    fn terminated() {
        let parse = |s| matcher::terminated(mock()).parse(s);

        assert_eq!(parse(MOCK_TOKEN).unwrap().0, MockResult);

        let with_semicolon = format!("{};", MOCK_TOKEN);
        assert_eq!(parse(with_semicolon.as_str()).unwrap().0, MockResult);
    }

    #[test]
    fn symbol() {
        let parse = |s| matcher::symbol('+').parse(s);

        assert_eq!(parse("+").unwrap().0, ());
        assert_eq!(parse("+ ").unwrap().0, ());
        assert_eq!(parse("-"), Err(StringStreamError::UnexpectedParse));
    }

    #[test]
    fn glyph() {
        let parse = |s| matcher::glyph("&&").parse(s);

        assert_eq!(parse("&&").unwrap().0, ());
        assert_eq!(parse("& & ").unwrap().0, ());
        assert_eq!(parse("||"), Err(StringStreamError::UnexpectedParse));
    }

    #[test]
    fn keyword() {
        let parse = |s| matcher::keyword("foo").parse(s);

        assert_eq!(parse("foo").unwrap().0, ());
        assert_eq!(parse("foo ").unwrap().0, ());
        assert_eq!(parse("bar"), Err(StringStreamError::UnexpectedParse));
    }

    #[test]
    fn identifier() {
        let parse = |s| matcher::identifier(parser::char::char('$')).parse(s);

        assert_eq!(parse("$foo_").unwrap().0, String::from("$foo_"));
        assert_eq!(parse("$").unwrap().0, String::from("$"));
        assert_eq!(parse(""), Err(StringStreamError::Eoi));
    }

    #[test]
    fn standard_identifier() {
        let parse = |s| matcher::standard_identifier().parse(s);

        assert_eq!(parse("foo").unwrap().0, String::from("foo"));
        assert_eq!(parse("_foo").unwrap().0, String::from("_foo"));
    }

    #[test]
    fn primitive_nil() {
        let parse = |s| primitive::primitive().parse(s);

        assert_eq!(parse("nil").unwrap().0, Primitive::Nil);
    }

    #[test]
    fn primitive_boolean_true() {
        let parse = |s| primitive::primitive().parse(s);

        assert_eq!(parse("true").unwrap().0, Primitive::Boolean(true));
    }

    #[test]
    fn primitive_boolean_false() {
        let parse = |s| primitive::primitive().parse(s);

        assert_eq!(parse("false").unwrap().0, Primitive::Boolean(false));
    }

    #[test]
    fn primitive_integer() {
        let parse = |s| primitive::primitive().parse(s);

        assert_eq!(parse("0").unwrap().0, Primitive::Integer(0));
        assert_eq!(parse("123").unwrap().0, Primitive::Integer(123));
    }

    #[test]
    fn primitive_float() {
        let parse = |s| primitive::primitive().parse(s);

        assert_eq!(parse("123.456").unwrap().0, Primitive::Float(123.456, 6));
    }

    #[test]
    fn primitive_string() {
        let parse = |s| primitive::primitive().parse(s);

        assert_eq!(
            parse("\"foo\"").unwrap().0,
            Primitive::String(String::from("foo"))
        );
    }

    #[test]
    fn expression_primitive() {
        let parse = |s| expression::expression().parse(s);

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
        let parse = |s| expression::expression().parse(s);

        assert_eq!(
            parse("foo").unwrap().0,
            Expression::Identifier(String::from("foo"))
        );
    }

    #[test]
    fn expression_group() {
        let parse = |s| expression::expression().parse(s);

        assert_eq!(
            parse("(nil)").unwrap().0,
            Expression::Group(Box::new(Expression::Primitive(Primitive::Nil)))
        );
    }

    #[test]
    fn expression_closure() {
        let parse = |s| expression::expression().parse(s);

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
        let parse = |s| expression::expression().parse(s);

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
        let parse = |s| expression::expression().parse(s);

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
        let parse = |s| expression::expression().parse(s);

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
        let parse = |s| expression::expression().parse(s);

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
            Expression::FunctionCall(Box::new(Expression::Primitive(Primitive::Nil)), Vec::new())
        );
    }

    #[test]
    fn expression_style() {
        let parse = |s| expression::expression().parse(s);

        assert_eq!(
            parse("style { foo: nil }").unwrap().0,
            Expression::Style(vec![(
                String::from("foo"),
                Expression::Primitive(Primitive::Nil)
            )])
        );
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

    #[test]
    fn statement_effect() {
        let parse = |s| statement::statement(mock).parse(s);

        let input = format!("{};", MOCK_TOKEN);
        assert_eq!(
            parse(input.as_str()).unwrap().0,
            Statement::Effect(MockResult)
        );
    }

    #[test]
    fn statement_variable() {
        let parse = |s| statement::statement(mock).parse(s);

        let input = format!("let foo = {};", MOCK_TOKEN);
        assert_eq!(
            parse(input.as_str()).unwrap().0,
            Statement::Variable(String::from("foo"), MockResult)
        );
    }
}
