open Kore;

module A = AST.Result;
module U = Util.ResultUtil;

let suite =
  "Library.AST"
  >::: [
    "Dump.pp() - nil primitive"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=nil_const />
    <Constant@0.0 type=nil>
      <Primitive@0.0 type=nil value=Nil />
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.nil_const |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - integer primitive"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=int_const />
    <Constant@0.0 type=integer>
      <Primitive@0.0 type=integer value=Number(123) />
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.int_const |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - float primitive"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=float_const />
    <Constant@0.0 type=float>
      <Primitive@0.0 type=float value=Number(123.000) />
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.float_const |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - boolean primitive"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=bool_const />
    <Constant@0.0 type=boolean>
      <Primitive@0.0 type=boolean value=Boolean(true) />
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.bool_const |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - string primitive"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=string_const />
    <Constant@0.0 type=string>
      <Primitive@0.0 type=string value=String(\"foo\") />
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.string_const |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - identifier expression"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=identifier_const />
    <Constant@0.0 type=integer>
      <Identifier@0.0 type=integer value=foo />
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.identifier_const |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - JSX expression"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=jsx_const />
    <Constant@0.0 type=element>
      <JSX@0.0 type=element>
        <Tag>
          <Name@0.0 value=Foo />
          <Attributes />
          <Children />
        </Tag>
      </JSX@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.jsx_const |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - group expression"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=group_const />
    <Constant@0.0 type=integer>
      <Group@0.0 type=integer>
        <Primitive@0.0 type=integer value=Number(123) />
      </Group@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.group_const |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - closure expression"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=closure_const />
    <Constant@0.0 type=boolean>
      <Closure@0.0 type=boolean>
        <Statement@0.0 type=nil>
          <Variable@0.0 type=nil>
            <Name@0.0 value=foo />
            <Primitive@0.0 type=integer value=Number(123) />
          </Variable@0.0>
        </Statement@0.0>
        <Statement@0.0 type=nil>
          <Variable@0.0 type=nil>
            <Name@0.0 value=bar />
            <Primitive@0.0 type=nil value=Nil />
          </Variable@0.0>
        </Statement@0.0>
        <Statement@0.0 type=boolean>
          <Expression@0.0 type=boolean>
            <Primitive@0.0 type=boolean value=Boolean(false) />
          </Expression@0.0>
        </Statement@0.0>
      </Closure@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.closure_const |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - binary operation"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=and_bool_const />
    <Constant@0.0 type=boolean>
      <And@0.0 type=boolean>
        <LHS@0.0 type=boolean>
          <Primitive@0.0 type=boolean value=Boolean(true) />
        </LHS@0.0>
        <RHS@0.0 type=boolean>
          <Primitive@0.0 type=boolean value=Boolean(false) />
        </RHS@0.0>
      </And@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.and_bool_const |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - unary operation"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=negative_int_const />
    <Constant@0.0 type=integer>
      <Negative@0.0 type=integer>
        <Primitive@0.0 type=integer value=Number(123) />
      </Negative@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.negative_int_const |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - jsx"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=complex_jsx_const />
    <Constant@0.0 type=element>
      <JSX@0.0 type=element>
        <Tag>
          <Name@0.0 value=Foo />
          <Attributes>
            <ID@0.0>
              <Name@0.0 value=bar />
            </ID@0.0>
            <Class@0.0>
              <Name@0.0 value=fizz />
            </Class@0.0>
            <Property@0.0>
              <Name@0.0 value=buzz />
            </Property@0.0>
          </Attributes>
          <Children>
            <Node@0.0>
              <Tag>
                <Name@0.0 value=Bar />
                <Attributes />
                <Children />
              </Tag>
            </Node@0.0>
            <InlineExpr@0.0>
              <Primitive@0.0 type=nil value=Nil />
            </InlineExpr@0.0>
            <Text@0.0 value=fizzbuzz />
            <Node@0.0>
              <Fragment />
            </Node@0.0>
          </Children>
        </Tag>
      </JSX@0.0>
    </Constant@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.complex_jsx_const |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - inline function"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=inline_function />
    <Function@0.0 type=(integer, integer) -> integer>
      <Arguments>
        <Argument@0.0 type=integer>
          <Name@0.0 value=foo />
        </Argument@0.0>
        <Argument@0.0 type=integer>
          <Primitive@0.0 type=integer value=Number(3) />
          <Name@0.0 value=bar />
        </Argument@0.0>
      </Arguments>
      <Body>
        <Add@0.0 type=integer>
          <LHS@0.0 type=integer>
            <Identifier@0.0 type=integer value=foo />
          </LHS@0.0>
          <RHS@0.0 type=integer>
            <Identifier@0.0 type=integer value=bar />
          </RHS@0.0>
        </Add@0.0>
      </Body>
    </Function@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.inline_function |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - multiline function"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Declaration@0.0>
    <NamedExport@0.0 value=multiline_function />
    <Function@0.0 type=(integer, integer) -> integer>
      <Arguments />
      <Body>
        <Closure@0.0 type=integer>
          <Statement@0.0 type=nil>
            <Variable@0.0 type=nil>
              <Name@0.0 value=zip />
              <Primitive@0.0 type=integer value=Number(3) />
            </Variable@0.0>
          </Statement@0.0>
          <Statement@0.0 type=nil>
            <Variable@0.0 type=nil>
              <Name@0.0 value=zap />
              <Primitive@0.0 type=integer value=Number(4) />
            </Variable@0.0>
          </Statement@0.0>
          <Statement@0.0 type=integer>
            <Expression@0.0 type=integer>
              <Mult@0.0 type=integer>
                <LHS@0.0 type=integer>
                  <Identifier@0.0 type=integer value=zip />
                </LHS@0.0>
                <RHS@0.0 type=integer>
                  <Identifier@0.0 type=integer value=zap />
                </RHS@0.0>
              </Mult@0.0>
            </Expression@0.0>
          </Statement@0.0>
        </Closure@0.0>
      </Body>
    </Function@0.0>
  </Declaration@0.0>
</AST>",
          Fixtures.multiline_function |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - main import"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Import@0.0 namespace=main_import>
    <MainImport@0.0>
      <Name@0.0 value=Foo />
    </MainImport@0.0>
  </Import@0.0>
</AST>",
          Fixtures.main_import |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - named import"
    >: (
      () =>
        Assert.string(
          "<AST>
  <Import@0.0 namespace=named_import>
    <NamedImport@0.0>
      <Name@0.0 value=foo />
    </NamedImport@0.0>
  </Import@0.0>
</AST>",
          Fixtures.named_import |> ~@A.Dump.pp,
        )
    ),
    "Dump.pp() - program"
    >: (() => Assert.string("<AST />", [] |> ~@A.Dump.pp)),
  ];
