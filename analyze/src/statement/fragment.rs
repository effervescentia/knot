use crate::{context::NodeContext, fragment::Fragment, register::ToFragment};
use lang::ast::StatementNodeValue;

impl<R> ToFragment for StatementNodeValue<R, NodeContext>
where
    R: Clone,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::Statement(self.map(&|x| *x.node().id()))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        context::NodeContext, fragment::Fragment, register::ToFragment, test::fixture as f,
    };
    use lang::ast::{Expression, Primitive, Statement};

    #[test]
    fn expression() {
        assert_eq!(
            Statement::Expression(f::n::xc(
                Expression::Primitive(Primitive::Nil),
                NodeContext::new(0, vec![0])
            ))
            .to_fragment(),
            Fragment::Statement(Statement::Expression(0))
        );
    }

    #[test]
    fn variable() {
        assert_eq!(
            Statement::Variable(
                String::from("foo"),
                f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )
            )
            .to_fragment(),
            Fragment::Statement(Statement::Variable(String::from("foo"), 0))
        );
    }
}
