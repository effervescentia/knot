use crate::javascript::Expression;
use kore::format::{Block, Indented, SeparateEach, TerminateEach};
use std::fmt::{Display, Formatter};

/// [JavaScript escape characters](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Lexical_grammar#string_literals)
fn escape_string(s: &str) -> String {
    s.replace('\\', "\\\\")
        .replace('"', "\\\"")
        .replace('\n', "\\n")
        .replace('\t', "\\t")
        .replace('\r', "\\r")
        .replace('\u{007F}', "\\b")
        .replace('\u{000C}', "\\f")
        .replace('\u{000B}', "\\v")
}

impl Display for Expression {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Null => write!(f, "null"),

            Self::Boolean(true) => write!(f, "true"),
            Self::Boolean(false) => write!(f, "false"),

            Self::Number(x) => write!(f, "{x}"),

            Self::String(x) => write!(f, "\"{escaped}\"", escaped = escape_string(x)),

            Self::Identifier(x) => write!(f, "{x}"),

            Self::Group(x) => write!(f, "({x})"),

            Self::UnaryOperation(op, x) => write!(f, "{op}{x}"),

            Self::BinaryOperation(op, lhs, rhs) => write!(f, "{lhs} {op} {rhs}"),

            Self::DotAccess(lhs, rhs) => write!(f, "{lhs}.{rhs}"),

            Self::FunctionCall(x, arguments) => {
                write!(
                    f,
                    "{x}({arguments})",
                    arguments = SeparateEach(", ", arguments)
                )
            }

            Self::Array(xs) => {
                write!(f, "[{items}]", items = SeparateEach(", ", xs))
            }

            Self::Object(xs) => {
                write!(
                    f,
                    "{{{properties}}}",
                    properties = Indented(Block(TerminateEach(
                        ",\n",
                        &xs.iter().map(|(key, value)| Property(key, value)).collect()
                    )))
                )
            }

            Self::Function(name, parameters, statements) => {
                write!(
                    f,
                    "function{name}({parameters}) {{{statements}}}",
                    name = name.as_ref().map(|x| format!(" {x}")).unwrap_or_default(),
                    parameters = SeparateEach(", ", parameters),
                    statements = Indented(Block(TerminateEach("\n", statements))),
                )
            }

            Self::Closure(xs) => {
                write!(
                    f,
                    "{iife}",
                    iife = Self::FunctionCall(
                        Box::new(Self::Group(Box::new(Self::Function(
                            None,
                            vec![],
                            xs.clone()
                        )))),
                        vec![],
                    )
                )
            }
        }
    }
}

struct Property<'a>(&'a String, &'a Expression);

impl<'a> Display for Property<'a> {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(f, "{key}: {value}", key = self.0, value = self.1)
    }
}

#[cfg(test)]
mod tests {
    use crate::javascript::{Expression, Statement};
    use kore::str;

    #[test]
    fn null() {
        assert_eq!(Expression::Null.to_string(), "null");
    }

    #[test]
    fn boolean() {
        assert_eq!(Expression::Boolean(true).to_string(), "true");
        assert_eq!(Expression::Boolean(false).to_string(), "false");
    }

    #[test]
    fn number() {
        assert_eq!(Expression::Number(str!("123")).to_string(), "123");
        assert_eq!(Expression::Number(str!("45.67")).to_string(), "45.67");
    }

    #[test]
    fn string() {
        assert_eq!(Expression::String(str!("foo")).to_string(), "\"foo\"");
    }

    #[test]
    fn escaped_string() {
        assert_eq!(
            Expression::String(str!("\\, \", \n, \t, \r, \u{007F}, \u{000C}, \u{000B}"))
                .to_string(),
            "\"\\\\, \\\", \\n, \\t, \\r, \\b, \\f, \\v\""
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(Expression::Identifier(str!("foo")).to_string(), "foo");
    }

    #[test]
    fn group() {
        assert_eq!(
            Expression::Group(Box::new(Expression::Null)).to_string(),
            "(null)"
        );
    }

    #[test]
    fn unary_operation() {
        assert_eq!(
            Expression::UnaryOperation("@", Box::new(Expression::Null)).to_string(),
            "@null"
        );
    }

    #[test]
    fn binary_operation() {
        assert_eq!(
            Expression::BinaryOperation(
                "%",
                Box::new(Expression::Null),
                Box::new(Expression::Null)
            )
            .to_string(),
            "null % null"
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            Expression::DotAccess(Box::new(Expression::Null), str!("foo")).to_string(),
            "null.foo"
        );
    }

    #[test]
    fn function_call() {
        assert_eq!(
            Expression::FunctionCall(Box::new(Expression::Null), vec![Expression::Null])
                .to_string(),
            "null(null)"
        );
    }

    #[test]
    fn array() {
        assert_eq!(
            Expression::Array(vec![Expression::Null, Expression::Null]).to_string(),
            "[null, null]"
        );
    }

    #[test]
    fn object() {
        assert_eq!(
            Expression::Object(vec![
                (str!("foo"), Expression::Null),
                (str!("bar"), Expression::Null)
            ])
            .to_string(),
            "{
  foo: null,
  bar: null,
}"
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            Expression::Function(Some(str!("foo")), vec![str!("bar"), str!("fizz")], vec![])
                .to_string(),
            "function foo(bar, fizz) {}"
        );
    }

    #[test]
    fn anonymous_function() {
        assert_eq!(
            Expression::Function(None, vec![str!("bar"), str!("fizz")], vec![]).to_string(),
            "function(bar, fizz) {}"
        );
    }

    #[test]
    fn function_statements() {
        assert_eq!(
            Expression::Function(None, vec![], vec![Statement::Return(None)]).to_string(),
            "function() {
  return null;
}"
        );
    }

    #[test]
    fn closure() {
        assert_eq!(
            Expression::Closure(vec![Statement::Return(None)]).to_string(),
            "(function() {
  return null;
})()"
        );
    }
}
