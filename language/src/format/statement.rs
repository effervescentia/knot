use crate::ast::{Statement, StatementNode};
use std::fmt::{Display, Formatter};

impl<R, C> Display for StatementNode<R, C>
where
    R: Clone,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self.node().value() {
            Statement::Expression(x) => write!(f, "{x};"),
            Statement::Variable(name, x) => write!(f, "let {name} = {x};"),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{Expression, Primitive, Statement},
        test::fixture as f,
    };

    #[test]
    fn expression() {
        assert_eq!(
            f::n::s(Statement::Expression(f::n::x(Expression::Primitive(
                Primitive::Nil
            ))))
            .to_string(),
            "nil;"
        );
    }

    #[test]
    fn variable() {
        assert_eq!(
            f::n::s(Statement::Variable(
                String::from("x"),
                f::n::x(Expression::Primitive(Primitive::Nil))
            ))
            .to_string(),
            "let x = nil;"
        );
    }
}
