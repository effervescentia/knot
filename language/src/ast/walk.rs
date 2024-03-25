use crate::Range;

#[derive(Clone)]
pub struct Span<Value>(pub Value, pub Range);

pub trait IntoSpan<Value> {
    fn into_span(self) -> Span<Value>;
}

impl<Value> IntoSpan<Value> for Span<Value> {
    fn into_span(self) -> Self {
        self
    }
}

pub trait Visit {
    type Binding;
    type Expression;
    type Statement;
    type Component;
    type TypeExpression;
    type Parameter;
    type Declaration;
    type Import;
    type Module;

    fn binding(self, x: super::Binding, r: Range) -> (Self::Binding, Self);

    fn expression(
        self,
        x: super::Expression<Self::Expression, Self::Statement, Self::Component>,
        r: Range,
    ) -> (Self::Expression, Self);

    fn statement(self, x: super::Statement<Self::Expression>, r: Range) -> (Self::Statement, Self);

    fn component(
        self,
        x: super::Component<Self::Component, Self::Expression>,
        r: Range,
    ) -> (Self::Component, Self);

    fn type_expression(
        self,
        x: super::TypeExpression<Self::TypeExpression>,
        r: Range,
    ) -> (Self::TypeExpression, Self);

    fn parameter(
        self,
        x: super::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        r: Range,
    ) -> (Self::Parameter, Self);

    #[allow(clippy::type_complexity)]
    fn declaration(
        self,
        x: super::Declaration<
            Self::Binding,
            Self::Expression,
            Self::TypeExpression,
            Self::Parameter,
            Self::Module,
        >,
        r: Range,
    ) -> (Self::Declaration, Self);

    fn import(self, x: super::Import, r: Range) -> (Self::Import, Self);

    fn module(
        self,
        x: super::Module<Self::Import, Self::Declaration>,
        r: Range,
    ) -> (Self::Module, Self);
}

pub trait Walk<Visitor>
where
    Visitor: Visit,
{
    type Output;

    fn walk(self, visitor: Visitor) -> (Self::Output, Visitor);
}

impl<Key, Target, Visitor> Walk<Visitor> for (Key, Target)
where
    Target: Walk<Visitor>,
    Visitor: Visit,
{
    type Output = (Key, Target::Output);

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (key, x) = self;
        let (x, v) = x.walk(v);

        ((key, x), v)
    }
}

impl<Target, Visitor> Walk<Visitor> for Option<Target>
where
    Target: Walk<Visitor>,
    Visitor: Visit,
{
    type Output = Option<Target::Output>;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        if let Some(x) = self {
            let (x, v) = x.walk(v);
            (Some(x), v)
        } else {
            (None, v)
        }
    }
}

impl<Target, Visitor> Walk<Visitor> for Vec<Target>
where
    Target: Walk<Visitor>,
    Visitor: Visit,
{
    type Output = Vec<Target::Output>;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.into_iter().fold((vec![], v), |(mut acc, v), x| {
            let (x, v) = x.walk(v);
            acc.push(x);
            (acc, v)
        })
    }
}
