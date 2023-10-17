use crate::{context::NodeContext, fragment::Fragment, register::ToFragment};
use lang::ast::ParameterNodeValue;

impl ToFragment for ParameterNodeValue<NodeContext> {
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::Parameter(self.map(&|x| *x.node().id(), &|x| *x.node().id()))
    }
}

#[cfg(test)]
mod tests {
    use crate::{context::NodeContext, fragment::Fragment, register::ToFragment};
    use lang::{
        ast::{Expression, Parameter, Primitive, TypeExpression},
        test::fixture as f,
    };

    #[test]
    fn parameter() {
        assert_eq!(
            Parameter::new(
                String::from("foo"),
                Some(f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                Some(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(1, vec![0])
                ))
            )
            .to_fragment(),
            Fragment::Parameter(Parameter::new(String::from("foo"), Some(0), Some(1)))
        );
    }
}
