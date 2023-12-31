use crate::javascript::{Expression, Statement};
use kore::format::SeparateEach;
use std::fmt::{Display, Formatter};

impl Display for Statement {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Expression(x) => match x {
                Expression::Function(..) => write!(f, "{x}"),

                _ => write!(f, "{x};"),
            },

            Self::Variable(name, x) => write!(f, "var {name} = {x};"),

            Self::Return(None) => write!(f, "return null;"),
            Self::Return(Some(x)) => write!(f, "return {x};"),

            Self::Export(name) => write!(f, "export {{ {name} }};"),

            Self::Assignment(lhs, rhs) => write!(f, "{lhs} = {rhs};"),

            Self::Import(namespace, imports) => {
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

            Self::ModuleImport(namespace, name) => {
                write!(f, "import * as {name} from \"{namespace}\";")
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
    use kore::str;

    #[test]
    fn expression() {
        assert_eq!(Statement::Expression(Expression::Null).to_string(), "null;");
    }

    #[test]
    fn variable() {
        assert_eq!(
            Statement::Variable(str!("foo"), Expression::Null).to_string(),
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
            Statement::Export(str!("foo")).to_string(),
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
