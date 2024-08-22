use crate::ast;
use std::fmt::{Display, Formatter};

impl<Expression> Display for ast::Statement<Expression>
where
    Expression: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Expression(x) => write!(f, "{x};"),
            Self::Variable(name, x) => write!(f, "let {name} = {x};"),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::ast;
    use kore::str;

    #[test]
    fn expression() {
        assert_eq!(
            ast::shape::Statement(ast::Statement::Expression(ast::shape::Expression(
                ast::Expression::Primitive(ast::Primitive::Nil)
            )))
            .to_string(),
            "nil;"
        );
    }

    #[test]
    fn variable() {
        assert_eq!(
            ast::shape::Statement(ast::Statement::Variable(
                str!("x"),
                ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
            ))
            .to_string(),
            "let x = nil;"
        );
    }
}
