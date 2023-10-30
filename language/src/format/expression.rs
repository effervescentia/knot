use crate::ast::{Expression, ExpressionNode, Primitive};
use kore::format::{indented, Block, Indented, SeparateEach, TerminateEach};
use std::fmt::{Display, Formatter, Write};

fn escape_string(s: &str) -> String {
    s.replace('\\', "\\\\")
        .replace('"', "\\\"")
        .replace('\n', "\\n")
        .replace('\t', "\\t")
        .replace('\r', "\\r")
}

impl<R, C> Display for ExpressionNode<R, C>
where
    R: Copy,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self.node().value() {
            Expression::Primitive(Primitive::Nil) => write!(f, "nil"),

            Expression::Primitive(Primitive::Boolean(true)) => write!(f, "true"),
            Expression::Primitive(Primitive::Boolean(false)) => write!(f, "false"),

            Expression::Primitive(Primitive::Integer(x)) => write!(f, "{x}"),

            Expression::Primitive(Primitive::Float(x, precision)) => {
                write!(f, "{x:.0$}", *precision as usize)
            }

            Expression::Primitive(Primitive::String(x)) => {
                write!(f, "\"{escaped}\"", escaped = escape_string(x))
            }

            Expression::Identifier(x) => write!(f, "{x}"),

            Expression::Group(x) => write!(f, "({x})"),

            Expression::Closure(xs) => {
                write!(
                    f,
                    "{{{statements}}}",
                    statements = Indented(Block(TerminateEach("\n", xs)))
                )
            }

            Expression::UnaryOperation(op, x) => write!(f, "{op}{x}"),

            Expression::BinaryOperation(op, lhs, rhs) => write!(f, "{lhs} {op} {rhs}"),

            Expression::DotAccess(lhs, rhs) => write!(f, "{lhs}.{rhs}"),

            Expression::FunctionCall(lhs, arguments) => {
                write!(
                    f,
                    "{lhs}({arguments})",
                    arguments = SeparateEach(", ", arguments)
                )
            }

            Expression::Style(xs) => write!(f, "style {{{rules}}}", rules = StyleRules(xs)),

            Expression::KSX(x) => write!(f, "{x}"),
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
    use crate::{
        ast::{BinaryOperator, Expression, Primitive, Statement, UnaryOperator, KSX},
        test::fixture as f,
    };

    #[test]
    fn nil() {
        assert_eq!(
            f::n::x(Expression::Primitive(Primitive::Nil)).to_string(),
            "nil"
        );
    }

    #[test]
    fn boolean() {
        assert_eq!(
            f::n::x(Expression::Primitive(Primitive::Boolean(true))).to_string(),
            "true"
        );
        assert_eq!(
            f::n::x(Expression::Primitive(Primitive::Boolean(false))).to_string(),
            "false"
        );
    }

    #[test]
    fn integer() {
        assert_eq!(
            f::n::x(Expression::Primitive(Primitive::Integer(123))).to_string(),
            "123"
        );
    }

    #[test]
    fn float() {
        assert_eq!(
            f::n::x(Expression::Primitive(Primitive::Float(123.45, 2))).to_string(),
            "123.45"
        );
    }

    #[test]
    fn string() {
        assert_eq!(
            f::n::x(Expression::Primitive(Primitive::String(String::from(
                "foo"
            ))))
            .to_string(),
            "\"foo\""
        );
        assert_eq!(
            f::n::x(Expression::Primitive(Primitive::String(String::from(
                "quotes (\")"
            ))))
            .to_string(),
            "\"quotes (\\\")\""
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            f::n::x(Expression::Identifier(String::from("foo"))).to_string(),
            "foo"
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            f::n::x(Expression::Group(Box::new(f::n::x(Expression::Primitive(
                Primitive::Nil
            )))))
            .to_string(),
            "(nil)"
        );
    }

    #[test]
    fn empty_closure() {
        assert_eq!(f::n::x(Expression::Closure(vec![])).to_string(), "{}");
    }

    #[test]
    fn closure_with_statements() {
        assert_eq!(
            f::n::x(Expression::Closure(vec![
                f::n::s(Statement::Variable(
                    String::from("x"),
                    f::n::x(Expression::Primitive(Primitive::Nil))
                )),
                f::n::s(Statement::Expression(f::n::x(Expression::Primitive(
                    Primitive::Nil
                ))))
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
            f::n::x(Expression::UnaryOperation(
                UnaryOperator::Not,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "!nil"
        );
    }

    #[test]
    fn unary_absolute_operation() {
        assert_eq!(
            f::n::x(Expression::UnaryOperation(
                UnaryOperator::Absolute,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "+nil"
        );
    }

    #[test]
    fn unary_negate_operation() {
        assert_eq!(
            f::n::x(Expression::UnaryOperation(
                UnaryOperator::Negate,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "-nil"
        );
    }

    #[test]
    fn binary_and_operation() {
        assert_eq!(
            f::n::x(Expression::BinaryOperation(
                BinaryOperator::And,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "nil && nil"
        );
    }

    #[test]
    fn binary_or_operation() {
        assert_eq!(
            f::n::x(Expression::BinaryOperation(
                BinaryOperator::Or,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "nil || nil"
        );
    }

    #[test]
    fn binary_equal_operation() {
        assert_eq!(
            f::n::x(Expression::BinaryOperation(
                BinaryOperator::Equal,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "nil == nil"
        );
    }

    #[test]
    fn binary_unequal_operation() {
        assert_eq!(
            f::n::x(Expression::BinaryOperation(
                BinaryOperator::NotEqual,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "nil != nil"
        );
    }

    #[test]
    fn binary_add_operation() {
        assert_eq!(
            f::n::x(Expression::BinaryOperation(
                BinaryOperator::Add,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "nil + nil"
        );
    }

    #[test]
    fn binary_subtract_operation() {
        assert_eq!(
            f::n::x(Expression::BinaryOperation(
                BinaryOperator::Subtract,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "nil - nil"
        );
    }

    #[test]
    fn binary_multiply_operation() {
        assert_eq!(
            f::n::x(Expression::BinaryOperation(
                BinaryOperator::Multiply,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "nil * nil"
        );
    }

    #[test]
    fn binary_divide_operation() {
        assert_eq!(
            f::n::x(Expression::BinaryOperation(
                BinaryOperator::Divide,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "nil / nil"
        );
    }

    #[test]
    fn binary_exponent_operation() {
        assert_eq!(
            f::n::x(Expression::BinaryOperation(
                BinaryOperator::Exponent,
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil)))
            ))
            .to_string(),
            "nil ^ nil"
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            f::n::x(Expression::DotAccess(
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                String::from("foo")
            ))
            .to_string(),
            "nil.foo"
        );
    }

    #[test]
    fn function_call_no_arguments() {
        assert_eq!(
            f::n::x(Expression::FunctionCall(
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                vec![]
            ))
            .to_string(),
            "nil()"
        );
    }

    #[test]
    fn function_call_with_arguments() {
        assert_eq!(
            f::n::x(Expression::FunctionCall(
                Box::new(f::n::x(Expression::Primitive(Primitive::Nil))),
                vec![
                    f::n::x(Expression::Primitive(Primitive::Nil)),
                    f::n::x(Expression::Primitive(Primitive::Nil))
                ]
            ))
            .to_string(),
            "nil(nil, nil)"
        );
    }

    #[test]
    fn empty_style() {
        assert_eq!(f::n::x(Expression::Style(vec![])).to_string(), "style {}");
    }

    #[test]
    fn style_with_rules() {
        assert_eq!(
            f::n::x(Expression::Style(vec![
                (
                    String::from("foo"),
                    f::n::x(Expression::Primitive(Primitive::Nil))
                ),
                (
                    String::from("bar"),
                    f::n::x(Expression::Primitive(Primitive::Nil))
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
            f::n::x(Expression::KSX(Box::new(f::n::kx(KSX::ClosedElement(
                String::from("foo"),
                vec![]
            )))))
            .to_string(),
            "<foo />"
        );
    }
}
