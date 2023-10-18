use crate::javascript::{Expression, Statement};
use format::SeparateEach;
use std::fmt::{Display, Formatter};

impl Display for Statement {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Statement::Expression(x) => match x {
                Expression::Function(..) => write!(f, "{x}"),

                _ => write!(f, "{x};"),
            },

            Statement::Variable(name, x) => write!(f, "var {name} = {x};"),

            Statement::Return(None) => write!(f, "return null;"),
            Statement::Return(Some(x)) => write!(f, "return {x};"),

            Statement::Export(name) => write!(f, "export {{ {name} }};"),

            Statement::Assignment(lhs, rhs) => write!(f, "{lhs} = {rhs};"),

            Statement::Import(namespace, imports) => {
                write!(
                    f,
                    "import {{ {imports} }} from \"{namespace}\";",
                    imports = SeparateEach(
                        ", ",
                        &imports
                            .iter()
                            .map(|(name, alias)| Import(name, alias))
                            .collect()
                    )
                )
            }
        }
    }
}

struct Import<'a>(&'a str, &'a Option<String>);

impl<'a> Display for Import<'a> {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self(name, Some(alias)) => write!(f, "{name} as {alias}"),

            Self(name, None) => write!(f, "{name}"),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::javascript::{Expression, Statement};

    #[test]
    fn expression() {
        assert_eq!(Statement::Expression(Expression::Null).to_string(), "null;");
    }

    #[test]
    fn variable() {
        assert_eq!(
            Statement::Variable(String::from("foo"), Expression::Null).to_string(),
            "var foo = null;"
        );
    }

    #[test]
    fn return_empty() {
        assert_eq!(Statement::Return(None).to_string(), "return null;");
    }

    #[test]
    fn return_value() {
        assert_eq!(
            Statement::Return(Some(Expression::Boolean(true))).to_string(),
            "return true;"
        );
    }

    #[test]
    fn export() {
        assert_eq!(
            Statement::Export(String::from("foo")).to_string(),
            "export { foo };"
        );
    }

    #[test]
    fn assignment() {
        assert_eq!(
            Statement::Assignment(Expression::Null, Expression::Null).to_string(),
            "null = null;"
        );
    }
}
