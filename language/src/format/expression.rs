use crate::ast;
use kore::format::{indented, Block, Indented, SeparateEach, TerminateEach};
use std::fmt::{Display, Formatter, Write};

fn escape_string(s: &str) -> String {
    s.replace('\\', "\\\\")
        .replace('"', "\\\"")
        .replace('\n', "\\n")
        .replace('\t', "\\t")
        .replace('\r', "\\r")
}

impl<Expression_, Statement, Component> Display
    for ast::Expression<Expression_, Statement, Component>
where
    Expression_: Display,
    Statement: Display,
    Component: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Primitive(ast::Primitive::Nil) => write!(f, "nil"),

            Self::Primitive(ast::Primitive::Boolean(true)) => write!(f, "true"),
            Self::Primitive(ast::Primitive::Boolean(false)) => write!(f, "false"),

            Self::Primitive(ast::Primitive::Integer(x)) => write!(f, "{x}"),

            Self::Primitive(ast::Primitive::Float(x, precision)) => {
                write!(f, "{x:.0$}", *precision as usize)
            }

            Self::Primitive(ast::Primitive::String(x)) => {
                write!(f, "\"{escaped}\"", escaped = escape_string(x))
            }

            Self::Identifier(x) => write!(f, "{x}"),

            Self::Group(x) => write!(f, "({x})"),

            Self::Closure(xs) => {
                write!(
                    f,
                    "{{{statements}}}",
                    statements = Indented(Block(TerminateEach("\n", xs)))
                )
            }

            Self::UnaryOperation(op, x) => write!(f, "{op}{x}"),

            Self::BinaryOperation(op, lhs, rhs) => write!(f, "{lhs} {op} {rhs}"),

            Self::PropertyAccess(lhs, rhs) => write!(f, "{lhs}.{rhs}"),

            Self::FunctionCall(lhs, arguments) => {
                write!(
                    f,
                    "{lhs}({arguments})",
                    arguments = SeparateEach(", ", arguments)
                )
            }

            Self::Style(xs) => write!(f, "style {{{rules}}}", rules = StyleRules(xs)),

            Self::Component(x) => write!(f, "{x}"),
        }
    }
}

struct StyleRules<'a, T>(&'a Vec<(String, T)>)
where
    T: Display;

impl<'a, T> Display for StyleRules<'a, T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        if self.0.is_empty() {
            Ok(())
        } else {
            let mut f = indented(f);

            self.0.iter().fold(writeln!(f), |acc, (key, value)| {
                acc.and_then(|_| writeln!(f, "{key}: {value},"))
            })
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::ast;
    use kore::str;

    #[test]
    fn nil() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)).to_string(),
            "nil"
        );
    }

    #[test]
    fn boolean() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Boolean(true)))
                .to_string(),
            "true"
        );
        assert_eq!(
            ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Boolean(false)))
                .to_string(),
            "false"
        );
    }

    #[test]
    fn integer() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Integer(123)))
                .to_string(),
            "123"
        );
    }

    #[test]
    fn float() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Float(123.45, 2)))
                .to_string(),
            "123.45"
        );
    }

    #[test]
    fn string() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::String(str!(
                "foo"
            ))))
            .to_string(),
            "\"foo\""
        );
        assert_eq!(
            ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::String(str!(
                "quotes (\")"
            ))))
            .to_string(),
            "\"quotes (\\\")\""
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::Identifier(str!("foo"))).to_string(),
            "foo"
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::Group(Box::new(ast::shape::Expression(
                ast::Expression::Primitive(ast::Primitive::Nil)
            ))))
            .to_string(),
            "(nil)"
        );
    }

    #[test]
    fn empty_closure() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::Closure(vec![])).to_string(),
            "{}"
        );
    }

    #[test]
    fn closure_with_statements() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::Closure(vec![
                ast::shape::Statement(ast::Statement::Variable(
                    str!("x"),
                    ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
                )),
                ast::shape::Statement(ast::Statement::Expression(ast::shape::Expression(
                    ast::Expression::Primitive(ast::Primitive::Nil)
                )))
            ]))
            .to_string(),
            "{
  let x = nil;
  nil;
}"
        );
    }

    #[test]
    fn unary_not_operation() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::UnaryOperation(
                ast::UnaryOperator::Not,
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "!nil"
        );
    }

    #[test]
    fn unary_absolute_operation() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::UnaryOperation(
                ast::UnaryOperator::Absolute,
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "+nil"
        );
    }

    #[test]
    fn unary_negate_operation() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::UnaryOperation(
                ast::UnaryOperator::Negate,
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "-nil"
        );
    }

    #[test]
    fn binary_and_operation() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::BinaryOperation(
                ast::BinaryOperator::And,
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                ))),
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "nil && nil"
        );
    }

    #[test]
    fn binary_or_operation() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::BinaryOperation(
                ast::BinaryOperator::Or,
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                ))),
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "nil || nil"
        );
    }

    #[test]
    fn binary_equal_operation() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::BinaryOperation(
                ast::BinaryOperator::Equal,
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                ))),
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "nil == nil"
        );
    }

    #[test]
    fn binary_unequal_operation() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::BinaryOperation(
                ast::BinaryOperator::NotEqual,
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                ))),
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "nil != nil"
        );
    }

    #[test]
    fn binary_add_operation() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::BinaryOperation(
                ast::BinaryOperator::Add,
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                ))),
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "nil + nil"
        );
    }

    #[test]
    fn binary_subtract_operation() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::BinaryOperation(
                ast::BinaryOperator::Subtract,
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                ))),
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "nil - nil"
        );
    }

    #[test]
    fn binary_multiply_operation() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::BinaryOperation(
                ast::BinaryOperator::Multiply,
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                ))),
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "nil * nil"
        );
    }

    #[test]
    fn binary_divide_operation() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::BinaryOperation(
                ast::BinaryOperator::Divide,
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                ))),
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "nil / nil"
        );
    }

    #[test]
    fn binary_exponent_operation() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::BinaryOperation(
                ast::BinaryOperator::Exponent,
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                ))),
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                )))
            ))
            .to_string(),
            "nil ^ nil"
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::PropertyAccess(
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                ))),
                str!("foo")
            ))
            .to_string(),
            "nil.foo"
        );
    }

    #[test]
    fn function_call_no_arguments() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::FunctionCall(
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                ))),
                vec![]
            ))
            .to_string(),
            "nil()"
        );
    }

    #[test]
    fn function_call_with_arguments() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::FunctionCall(
                Box::new(ast::shape::Expression(ast::Expression::Primitive(
                    ast::Primitive::Nil
                ))),
                vec![
                    ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                    ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
                ]
            ))
            .to_string(),
            "nil(nil, nil)"
        );
    }

    #[test]
    fn empty_style() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::Style(vec![])).to_string(),
            "style {}"
        );
    }

    #[test]
    fn style_with_rules() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::Style(vec![
                (
                    str!("foo"),
                    ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
                ),
                (
                    str!("bar"),
                    ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
                ),
            ]))
            .to_string(),
            "style {
  foo: nil,
  bar: nil,
}"
        );
    }

    #[test]
    fn ksx() {
        assert_eq!(
            ast::shape::Expression(ast::Expression::Component(Box::new(ast::shape::Component(
                ast::Component::ClosedElement(str!("foo"), vec![])
            ))))
            .to_string(),
            "<foo />"
        );
    }
}
