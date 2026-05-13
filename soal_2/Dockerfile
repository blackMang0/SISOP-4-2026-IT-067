FROM ubuntu:latest

RUN apt update && apt install -y \
    gcc \
    make \
    fuse3 \
    libfuse3-dev

WORKDIR /app

COPY . .

EXPOSE 8080

CMD ["./server"]
