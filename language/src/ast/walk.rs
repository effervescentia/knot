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

pub trait Visit: Sized {
    type Binding;
    type Expression;
    type Statement;
    type Component;
    type TypeExpression;
    type Parameter;
    type Declaration;
    type Import;
    type Module;

    fn scoped<T, F>(self, f: F) -> (T, Self)
    where
        F: FnOnce(Self) -> (T, Self),
    {
        f(self)
    }

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

impl<Target, Visitor> Walk<Visitor> for (String, Target)
where
    Target: Walk<Visitor>,
    Visitor: Visit,
{
    type Output = (String, Target::Output);

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

pub trait WalkEach<Visitor>
where
    Visitor: Visit,
{
    type Output;

    fn walk_each(self, visitor: Visitor) -> (Self::Output, Visitor);
}

impl<T1, T2, Visitor> WalkEach<Visitor> for (T1, T2)
where
    T1: Walk<Visitor>,
    T2: Walk<Visitor>,
    Visitor: Visit,
{
    type Output = (T1::Output, T2::Output);

    fn walk_each(self, v: Visitor) -> (Self::Output, Visitor) {
        v.scoped(|v| {
            let (r1, v) = self.0.walk(v);
            let (r2, v) = self.1.walk(v);
            ((r1, r2), v)
        })
    }
}

impl<T1, T2, T3, Visitor> WalkEach<Visitor> for (T1, T2, T3)
where
    T1: Walk<Visitor>,
    T2: Walk<Visitor>,
    T3: Walk<Visitor>,
    Visitor: Visit,
{
    type Output = (T1::Output, T2::Output, T3::Output);

    fn walk_each(self, v: Visitor) -> (Self::Output, Visitor) {
        v.scoped(|v| {
            let (r1, v) = self.0.walk(v);
            let (r2, v) = self.1.walk(v);
            let (r3, v) = self.2.walk(v);
            ((r1, r2, r3), v)
        })
    }
}

impl<T1, T2, T3, T4, Visitor> WalkEach<Visitor> for (T1, T2, T3, T4)
where
    T1: Walk<Visitor>,
    T2: Walk<Visitor>,
    T3: Walk<Visitor>,
    T4: Walk<Visitor>,
    Visitor: Visit,
{
    type Output = (T1::Output, T2::Output, T3::Output, T4::Output);

    fn walk_each(self, v: Visitor) -> (Self::Output, Visitor) {
        v.scoped(|v| {
            let (r1, v) = self.0.walk(v);
            let (r2, v) = self.1.walk(v);
            let (r3, v) = self.2.walk(v);
            let (r4, v) = self.3.walk(v);
            ((r1, r2, r3, r4), v)
        })
    }
}
