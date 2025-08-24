# Step 1: Build the binary in a controlled environment
FROM golang:1.21-alpine AS builder

WORKDIR /app

# Copy dependency files and download dependencies
COPY go.mod go.sum ./
RUN go mod download

# Copy the source code
COPY . .

# Build the application, enabling CGO for SQLite and creating a static binary
RUN CGO_ENABLED=1 GOOS=linux go build -a -installsuffix cgo -o /edge-processor ./cmd/main.go

# Step 2: Create the final, small image for production
FROM alpine:latest

# Install SQLite CLI tool for easier debugging and any necessary certificates
RUN apk --no-cache add ca-certificates sqlite

WORKDIR /app

# Copy the compiled binary from the builder stage
COPY --from=builder /edge-processor .

# Copy the configuration file
COPY config.yml .

# Copy the web UI files
COPY web ./web

# This directory will be mounted as a volume to persist the database
RUN mkdir /data

# Expose the API port
EXPOSE 8080

# The command to run the application
CMD ["./edge-processor"]