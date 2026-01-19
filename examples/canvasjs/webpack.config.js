const path = require("path");
const webpack = require("webpack");
const HtmlWebpackPlugin = require("html-webpack-plugin");
const { CleanWebpackPlugin } = require("clean-webpack-plugin");
const MiniCssExtractPlugin = require("mini-css-extract-plugin");
const CopyPlugin = require("copy-webpack-plugin");

module.exports = (env, argv) => {
  const isProd = argv.mode === "production";
  return {
    entry: "./src/index.ts",
    output: {
      path: path.resolve(__dirname, "build"),
      filename: "app.[chunkhash].js",
    },
    devtool: isProd ? undefined : "inline-source-map",
    devServer: {
      watchFiles: ["src/*.css"],
      hot: false,
      liveReload: true,
    },
    module: {
      rules: [
        {
          test: /\.css$/,
          use: [MiniCssExtractPlugin.loader, "css-loader"],
        },
        {
          test: /\.ts?$/,
          use: {
            loader: "ts-loader",
          },
          exclude: /node_modules/,
        },
      ],
    },
    resolve: {
      extensions: [".tsx", ".ts", ".js"],
    },
    plugins: [
      // new CopyPlugin({
      //   patterns: [
      //     // { from: "public/manifest.json", to: "." },
      //     // { from: "public/favicon.svg", to: "." },
      //   ],
      // }),
      new HtmlWebpackPlugin({
        template: "public/index.html",
      }),
      new webpack.DefinePlugin({
        ISOLATED: argv.env.isolated,
      }),
      new MiniCssExtractPlugin({
        filename: "styles.[chunkhash].css",
      }),
      isProd ? new CleanWebpackPlugin() : undefined,
    ].filter((x) => x),
  };
};
