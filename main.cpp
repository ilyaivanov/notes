
void PaintWindow() {
  StretchDIBits(windowDc, 0, 0, canvas.width, canvas.height, 0, 0, canvas.width, canvas.height,
                canvas.pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

struct LineBreak {
  // Soft line break is a text overflow break due to a limited screen width.
  // Non-soft (hard) line break is via \n
  i8 isSoft;
  i32 textPos;
};

struct Buffer {
  c16* text;
  i32 textLen;
  i32 textCapacity;

  LineBreak* lines;
  i32 linesLen;
  i32 linesCapacity;

  i32 selectionStart;
  i32 cursor;
  i32 desiredOffset;
};

Buffer buffer;
void LoadFile(c16* path) {
  i64 size = GetMyFileSize(path);
  c8* file = (c8*)valloc(size);
  ReadFileInto(path, size, file);
  i32 wideCharsCount = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, file, size, 0, 0);
  c16* text = (c16*)valloc(wideCharsCount * sizeof(c16));
  MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, file, size, text, wideCharsCount);

  i32 bufferPos = 0;
  for (i32 i = 0; i < wideCharsCount; i++) {
    if (text[i] != '\r') {
      buffer.text[bufferPos] = text[i];
      bufferPos++;
    }
  }
  buffer.textLen = bufferPos;

  // RebuildLines();
}
